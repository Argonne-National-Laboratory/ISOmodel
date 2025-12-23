/*
 * SolarRadiation.cpp
 *
 * REFACTORING:
 * 1. Modern C++: Used std::clamp and std::array.
 * 2. Performance: Hoisted vector lookups in calculateAverages to reduce overhead in loop.
 * 3. Cleanup: Removed destructor implementation (defaulted in header).
 */

#include "SolarRadiation.hpp"
#include "EpwData.hpp"
#include <algorithm> // for std::clamp
#include <array>

namespace openstudio::isomodel {

    // Surface Azimuths in radians: S, SE, E, NE, N, NW, W, SW
    static constexpr std::array<double, 8> SurfaceAzimuths = { 
        0, -PI / 4, -PI / 2, -3 * PI / 4, PI, 3 * PI / 4, PI / 2, PI / 4 
    };

    SolarRadiation::SolarRadiation(TimeFrame* frame, EpwData* wdata, double tilt)
        : m_frame(frame), m_epwData(wdata)
    {
        // m_groundReflectance initialized in header
        m_surfaceTilt = tilt / 2.0;
        m_sinTilt = std::sin(m_surfaceTilt);
        m_cosTilt = std::cos(m_surfaceTilt);
        
        m_longitude = wdata->longitude() * (PI / 180.0);
        m_localMeridian = wdata->timezone() * 15.0 * (PI / 180.0);
        m_latitude = wdata->latitude() * (PI / 180.0);

        for (int i = 0; i < numVerticalSurfaces; ++i) {
            m_surfSin[i] = std::sin(SurfaceAzimuths[i]);
            m_surfCos[i] = std::cos(SurfaceAzimuths[i]);
        }

        m_eglobeFlat.assign(hoursInYear * numVerticalSurfaces, 0.0);
        
        m_monthlyDryBulbTemp.assign(monthsInYear, 0.0);
        m_monthlyDewPointTemp.assign(monthsInYear, 0.0);
        m_monthlyRelativeHumidity.assign(monthsInYear, 0.0);
        m_monthlyWindspeed.assign(monthsInYear, 0.0);
        m_monthlyGlobalHorizontalRadiation.assign(monthsInYear, 0.0);

        m_monthlySolarRadiation.assign(monthsInYear, std::vector<double>(numVerticalSurfaces, 0.0));
        m_hourlyDryBulbTemp.assign(monthsInYear, std::vector<double>(hoursInDay, 0.0));
        m_hourlyDewPointTemp.assign(monthsInYear, std::vector<double>(hoursInDay, 0.0));
        m_hourlyGlobalHorizontalRadiation.assign(monthsInYear, std::vector<double>(hoursInDay, 0.0));
    }

    // Destructor is defaulted in header

    std::vector<std::vector<double>> SolarRadiation::eglobe() {
        std::vector<std::vector<double>> legacy(hoursInYear, std::vector<double>(numVerticalSurfaces, 0.0));
        for (int i = 0; i < hoursInYear; ++i) {
            for (int s = 0; s < numVerticalSurfaces; ++s) {
                legacy[i][s] = m_eglobeFlat[i * numVerticalSurfaces + s];
            }
        }
        return legacy;
    }

    void SolarRadiation::Calculate() {
        calculateSurfaceSolarRadiation();
        calculateAverages();
    }

    void SolarRadiation::calculateSurfaceSolarRadiation() {
        const auto& dataMap = m_epwData->data();
        const std::vector<double>& vecEB = dataMap[EB];
        const std::vector<double>& vecED = dataMap[ED];

        int lastDay = -1;
        double eq = 0.0, dec = 0.0;
        double sinDec = 0.0, cosDec = 0.0;

        for (int i = 0; i < hoursInYear; i++) {
            
            // Recalculate daily variables only when day changes
            int currentDay = m_frame->YTD[i]; 
            if (currentDay != lastDay) {
                double rev = calculateRevolutionAngle(currentDay);
                eq = calculateEquationOfTime(rev);
                dec = calculateSolarDeclination(rev);
                sinDec = std::sin(dec);
                cosDec = std::cos(dec);
                lastDay = currentDay;
            }

            double ast = calculateApparentSolarTime(m_frame->Hour[i], eq);
            double sha = calculateSolarHourAngle(ast);
            double cosSha = std::cos(sha);
            double sinSha = std::sin(sha);
            
            // Replaced 'calculateSolarAltitude' with dot product
            double sinBeta = std::cos(m_latitude) * cosDec * cosSha + std::sin(m_latitude) * sinDec;
            
            // Clamp for safety before sqrt
            double clampedSinBeta = std::clamp(sinBeta, -1.0, 1.0);
            
            // cosBeta = sqrt(1 - sinBeta^2). 
            // If sun is below horizon (sinBeta < 0), we zero it out implicitly via the clamp/logic?
            // Original logic: if(sinBeta >= 1 || sinBeta <= -1) cosBeta = 0.
            double cosBeta = 0.0;
            if (std::abs(sinBeta) < 1.0) {
                cosBeta = std::sqrt(1.0 - clampedSinBeta * clampedSinBeta);
            }
            
            if (cosBeta < 1e-6) cosBeta = 1e-6; // Avoid division by zero

            double sinAz = sinSha * cosDec / cosBeta;
            double cosAz = (cosSha * cosDec * std::sin(m_latitude) - sinDec * std::cos(m_latitude)) / cosBeta;
            
            // Ground Reflected
            double ground = (vecEB[i] * sinBeta + vecED[i]) * m_groundReflectance * (1 - m_cosTilt) * 0.5;

            for (int s = 0; s < numVerticalSurfaces; s++) {
                // Incidence Angle
                double cosGamma = cosAz * m_surfCos[s] + sinAz * m_surfSin[s];
                double cosTheta = cosBeta * cosGamma * m_sinTilt + sinBeta * m_cosTilt;

                // Direct Beam
                double direct = (cosTheta > 0.0) ? vecEB[i] * cosTheta : 0.0;

                // Diffuse Angle of Incidence Factor
                double Y = std::max(0.45, 0.55 + 0.437 * cosTheta + 0.313 * cosTheta * cosTheta);
                
                // Total Diffuse
                double diff = (m_surfaceTilt > PI / 2) ? 
                               vecED[i] * Y * m_sinTilt : 
                               vecED[i] * (Y * m_sinTilt + m_cosTilt);

                // Total Irradiance
                m_eglobeFlat[i * numVerticalSurfaces + s] = direct + diff + ground;
            }
        }
    }

    void SolarRadiation::calculateAverages() {
        const auto& dataMap = m_epwData->data();
        
        // Hoist lookups out of loop for performance
        const auto& dbt = dataMap[DBT];
        const auto& dpt = dataMap[DPT];
        const auto& rh = dataMap[RH];
        const auto& wspd = dataMap[WSPD];
        const auto& egh = dataMap[EGH];

        int month = 0, midx = -1, cnt = 0;

        for (int i = 0; i < hoursInYear; i++, cnt++) {
            if (m_frame->Month[i] != month) {
                if (midx >= 0) calculateMonthAvg(midx, cnt);
                month = m_frame->Month[i];
                midx++;
                clearMonthlyAvg(midx);
                cnt = 0;
            }
            m_monthlyDryBulbTemp[midx] += dbt[i];
            m_monthlyDewPointTemp[midx] += dpt[i];
            m_monthlyRelativeHumidity[midx] += rh[i];
            m_monthlyWindspeed[midx] += wspd[i];
            m_monthlyGlobalHorizontalRadiation[midx] += egh[i];

            int h = m_frame->Hour[i];
            m_hourlyDryBulbTemp[midx][h] += dbt[i];
            m_hourlyDewPointTemp[midx][h] += dpt[i];
            m_hourlyGlobalHorizontalRadiation[midx][h] += egh[i];

            for (int s = 0; s < numVerticalSurfaces; s++) {
                m_monthlySolarRadiation[midx][s] += m_eglobeFlat[i * numVerticalSurfaces + s];
            }
        }
        calculateMonthAvg(midx, cnt);
    }

    void SolarRadiation::calculateMonthAvg(int midx, int cnt) {
        double inv = 1.0 / std::max(1, cnt);
        m_monthlyDryBulbTemp[midx] *= inv;
        m_monthlyDewPointTemp[midx] *= inv;
        m_monthlyRelativeHumidity[midx] *= inv;
        m_monthlyWindspeed[midx] *= inv;
        m_monthlyGlobalHorizontalRadiation[midx] *= inv;
        
        for (int s = 0; s < numVerticalSurfaces; s++) {
            m_monthlySolarRadiation[midx][s] *= inv;
        }

        double days = m_frame->monthLength(midx + 1);
        for (int h = 0; h < hoursInDay; h++) {
            m_hourlyDryBulbTemp[midx][h] /= days;
            m_hourlyDewPointTemp[midx][h] /= days;
            m_hourlyGlobalHorizontalRadiation[midx][h] /= days;
        }
    }

    void SolarRadiation::clearMonthlyAvg(int midx) {
        // Handled by initialization logic, but kept for interface consistency if used externally
    }

} // namespace openstudio::isomodel