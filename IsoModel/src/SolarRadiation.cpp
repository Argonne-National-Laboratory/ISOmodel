/*
 * SolarRadiation.cpp
 *
 * REFACTORING: PERFORMANCE OPTIMIZATION & DOCUMENTATION
 * 1. Memory: Removed heavy allocations from Constructor. Vectors are now lazy-loaded.
 * 2. Pre-calculation: Daily solar geometry (Declination/EOT) is computed once per year, 
 * not checked per hour. This fixes potential day-shift bugs.
 * 3. Flat Arrays: Replaced vector<vector> with flat vectors for cache locality.
 * 4. Documentation: Added equation references to ASHRAE Fundamentals 2013 and Duffie & Beckman.
 */

#include "SolarRadiation.hpp"
#include "EpwData.hpp"
#include <algorithm> // for std::clamp, std::max
#include <array>
#include <cmath>

namespace openstudio::isomodel {

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
        
        // Hoist latitude trig for loop performance
        m_sinLat = std::sin(m_latitude);
        m_cosLat = std::cos(m_latitude);

        for (int i = 0; i < numVerticalSurfaces; ++i) {
            m_surfSin[i] = std::sin(SurfaceAzimuths[i]);
            m_surfCos[i] = std::cos(SurfaceAzimuths[i]);
        }

        // Only allocate the essential output vector
        m_eglobeFlat.assign(hoursInYear * numVerticalSurfaces, 0.0);
        
        // OPTIMIZATION: Do NOT allocate the monthly/hourly average vectors here.
        // They are often unused by HourlyModel, saving 20+ malloc calls per object.
    }

    // Destructor is defaulted in header

    // Legacy support: Reconstructs 2D vector from flat storage
    std::vector<std::vector<double>> SolarRadiation::eglobe() {
        std::vector<std::vector<double>> legacy(hoursInYear, std::vector<double>(numVerticalSurfaces, 0.0));
        for (int i = 0; i < hoursInYear; ++i) {
            for (int s = 0; s < numVerticalSurfaces; ++s) {
                legacy[i][s] = m_eglobeFlat[i * numVerticalSurfaces + s];
            }
        }
        return legacy;
    }

    void SolarRadiation::Calculate(bool computeAverages) {
        calculateSurfaceSolarRadiation();
        if (computeAverages) {
            calculateAverages();
        }
    }

    void SolarRadiation::calculateSurfaceSolarRadiation() {
        const auto& dataMap = m_epwData->data();
        const std::vector<double>& vecEB = dataMap[EB]; // Beam Radiation
        const std::vector<double>& vecED = dataMap[ED]; // Diffuse Radiation

        // OPTIMIZATION: Pre-calculate daily solar geometry for all 365 days.
        // This removes the "if (newDay)" branch and math from the 8760 loop.
        // We initialize 0-366 to safely handle both 0-based and 1-based YTD indices.
        std::array<double, 367> dailyEqTime = {};
        std::array<double, 367> dailySinDec = {};
        std::array<double, 367> dailyCosDec = {};

        for (int d = 0; d <= 366; ++d) { 
             // Revolution Angle (B): Duffie & Beckman Eq 1.4.2
             double rev = calculateRevolutionAngle(d);
             // Equation of Time: ASHRAE Fundamentals 2013 Ch 14 Eq 1
             dailyEqTime[d] = calculateEquationOfTime(rev);
             // Declination: ASHRAE Fundamentals 2013 Ch 14 Eq 5 / D&B Eq 1.6.1b
             double dec = calculateSolarDeclination(rev);
             dailySinDec[d] = std::sin(dec);
             dailyCosDec[d] = std::cos(dec);
        }

        // Constant check hoisted out of loop
        const bool isTiltOver90 = (m_surfaceTilt > PI / 2.0);
        const double groundFactor = m_groundReflectance * (1 - m_cosTilt) * 0.5;

        // Flattened loop access
        double* pOutput = m_eglobeFlat.data();

        for (int i = 0; i < hoursInYear; i++) {
            
            // Fast lookup for daily values
            int currentDay = m_frame->YTD[i]; // 1-365 (or 0-365 depending on implementation)
            double eq = dailyEqTime[currentDay];
            double sinDec = dailySinDec[currentDay];
            double cosDec = dailyCosDec[currentDay];

            // Apparent Solar Time (AST): ASHRAE Fundamentals 2013 Ch 14 Eq 3
            double ast = calculateApparentSolarTime(m_frame->Hour[i], eq);
            // Hour Angle (H): ASHRAE Fundamentals 2013 Ch 14 Eq 4
            double sha = calculateSolarHourAngle(ast);
            double cosSha = std::cos(sha);
            double sinSha = std::sin(sha);
            
            // Solar Altitude (Beta): ASHRAE Fundamentals 2013 Ch 14 Eq 6 / D&B Eq 1.6.5
            // Optimization: Use pre-calculated sin/cos latitude
            double sinBeta = m_cosLat * cosDec * cosSha + m_sinLat * sinDec;
            
            // Clamp for safety before sqrt
            double clampedSinBeta = std::clamp(sinBeta, -1.0, 1.0);
            
            // cosBeta = sqrt(1 - sinBeta^2)
            double cosBeta = 0.0;
            if (std::abs(sinBeta) < 1.0) {
                cosBeta = std::sqrt(1.0 - clampedSinBeta * clampedSinBeta);
            }
            if (cosBeta < 1e-6) cosBeta = 1e-6; // Avoid division by zero

            // Solar Azimuth (Phi): ASHRAE Fundamentals 2013 Ch 14 Eq 7 / D&B Eq 1.6.6
            // Calculated via spherical trig helper components
            double sinAz = sinSha * cosDec / cosBeta;
            double cosAz = (cosSha * cosDec * m_sinLat - sinDec * m_cosLat) / cosBeta;
            
            // OPTIMIZATION: Hoist Surface-Independent Terms
            // This replaces re-calculating cosTheta terms inside the inner loop.
            // Based on D&B Eq 1.6.2 (Angle of Incidence) decomposition.
            double commonT1 = cosBeta * m_sinTilt;
            double termCos = commonT1 * cosAz;
            double termSin = commonT1 * sinAz;
            double termConstant = sinBeta * m_cosTilt;

            // Ground Reflected Radiation: ASHRAE Fundamentals 2013 Ch 14 Eq 23 (Isotropic)
            double ground = (vecEB[i] * sinBeta + vecED[i]) * groundFactor;
            double eb_i = vecEB[i];
            double ed_i = vecED[i];

            // Vectorization-friendly loop (no branches)
            for (int s = 0; s < numVerticalSurfaces; s++) {
                
                // Incidence Angle (Theta): ASHRAE Fundamentals 2013 Ch 14 Eq 8
                // Optimized: Uses pre-calculated terms
                double cosTheta = termCos * m_surfCos[s] + termSin * m_surfSin[s] + termConstant;

                // Direct Beam: ASHRAE Fundamentals 2013 Ch 14 Eq 9
                double direct = eb_i * std::max(0.0, cosTheta);

                // Diffuse Angle of Incidence Factor (Y): ASHRAE Fundamentals 2013 Ch 14 Eq 22
                // Specific polynomial for vertical surface diffuse correction
                double Y = std::max(0.45, 0.55 + 0.437 * cosTheta + 0.313 * cosTheta * cosTheta);
                
                // Total Diffuse: ASHRAE Fundamentals 2013 Ch 14 Eq 21
                double diff;
                if (isTiltOver90) {
                    diff = ed_i * Y * m_sinTilt;
                } else {
                    diff = ed_i * (Y * m_sinTilt + m_cosTilt);
                }

                // Total Irradiance = Direct + Diffuse + Ground
                *pOutput++ = direct + diff + ground;
            }
        }
    }

    void SolarRadiation::calculateAverages() {
        // OPTIMIZATION: Lazy Allocation
        // Only allocate these vectors if this function is actually called.
        // This saves significant memory/time for short-lived objects.
        if (m_monthlyDryBulbTemp.empty()) {
            m_monthlyDryBulbTemp.assign(monthsInYear, 0.0);
            m_monthlyDewPointTemp.assign(monthsInYear, 0.0);
            m_monthlyRelativeHumidity.assign(monthsInYear, 0.0);
            m_monthlyWindspeed.assign(monthsInYear, 0.0);
            m_monthlyGlobalHorizontalRadiation.assign(monthsInYear, 0.0);
            
            // Flattened allocations
            m_monthlySolarRadiation.assign(monthsInYear * numVerticalSurfaces, 0.0);
            m_hourlyDryBulbTemp.assign(monthsInYear * hoursInDay, 0.0);
            m_hourlyDewPointTemp.assign(monthsInYear * hoursInDay, 0.0);
            m_hourlyGlobalHorizontalRadiation.assign(monthsInYear * hoursInDay, 0.0);
        } else {
             // Reset if reused
             std::fill(m_monthlyDryBulbTemp.begin(), m_monthlyDryBulbTemp.end(), 0.0);
             std::fill(m_monthlyDewPointTemp.begin(), m_monthlyDewPointTemp.end(), 0.0);
             std::fill(m_monthlyRelativeHumidity.begin(), m_monthlyRelativeHumidity.end(), 0.0);
             std::fill(m_monthlyWindspeed.begin(), m_monthlyWindspeed.end(), 0.0);
             std::fill(m_monthlyGlobalHorizontalRadiation.begin(), m_monthlyGlobalHorizontalRadiation.end(), 0.0);
             std::fill(m_monthlySolarRadiation.begin(), m_monthlySolarRadiation.end(), 0.0);
             std::fill(m_hourlyDryBulbTemp.begin(), m_hourlyDryBulbTemp.end(), 0.0);
             std::fill(m_hourlyDewPointTemp.begin(), m_hourlyDewPointTemp.end(), 0.0);
             std::fill(m_hourlyGlobalHorizontalRadiation.begin(), m_hourlyGlobalHorizontalRadiation.end(), 0.0);
        }

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
            // Flat Indexing: month * 24 + h
            int hourIdx = midx * hoursInDay + h;
            m_hourlyDryBulbTemp[hourIdx] += dbt[i];
            m_hourlyDewPointTemp[hourIdx] += dpt[i];
            m_hourlyGlobalHorizontalRadiation[hourIdx] += egh[i];

            for (int s = 0; s < numVerticalSurfaces; s++) {
                // Flat Indexing: month * numSurfaces + s
                m_monthlySolarRadiation[midx * numVerticalSurfaces + s] += m_eglobeFlat[i * numVerticalSurfaces + s];
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
            m_monthlySolarRadiation[midx * numVerticalSurfaces + s] *= inv;
        }

        double days = m_frame->monthLength(midx + 1);
        double dayInv = 1.0 / days;
        
        int baseIdx = midx * hoursInDay;
        for (int h = 0; h < hoursInDay; h++) {
            m_hourlyDryBulbTemp[baseIdx + h] *= dayInv;
            m_hourlyDewPointTemp[baseIdx + h] *= dayInv;
            m_hourlyGlobalHorizontalRadiation[baseIdx + h] *= dayInv;
        }
    }

    void SolarRadiation::clearMonthlyAvg(int midx) {
        // Handled by initialization logic/reset, but kept for interface consistency
    }

    // --- Legacy Getters Re-implementation for Flat Vectors ---
    
    std::vector<std::vector<double>> SolarRadiation::monthlySolarRadiation() {
        std::vector<std::vector<double>> ret(monthsInYear, std::vector<double>(numVerticalSurfaces));
        for(int m=0; m<monthsInYear; ++m) {
            for(int s=0; s<numVerticalSurfaces; ++s) {
                ret[m][s] = m_monthlySolarRadiation[m * numVerticalSurfaces + s];
            }
        }
        return ret;
    }

    std::vector<std::vector<double>> SolarRadiation::hourlyDryBulbTemp() {
        std::vector<std::vector<double>> ret(monthsInYear, std::vector<double>(hoursInDay));
        for(int m=0; m<monthsInYear; ++m) {
            for(int h=0; h<hoursInDay; ++h) {
                ret[m][h] = m_hourlyDryBulbTemp[m * hoursInDay + h];
            }
        }
        return ret;
    }

    std::vector<std::vector<double>> SolarRadiation::hourlyDewPointTemp() {
        std::vector<std::vector<double>> ret(monthsInYear, std::vector<double>(hoursInDay));
        for(int m=0; m<monthsInYear; ++m) {
            for(int h=0; h<hoursInDay; ++h) {
                ret[m][h] = m_hourlyDewPointTemp[m * hoursInDay + h];
            }
        }
        return ret;
    }

    std::vector<std::vector<double>> SolarRadiation::hourlyGlobalHorizontalRadiation() {
        std::vector<std::vector<double>> ret(monthsInYear, std::vector<double>(hoursInDay));
        for(int m=0; m<monthsInYear; ++m) {
            for(int h=0; h<hoursInDay; ++h) {
                ret[m][h] = m_hourlyGlobalHorizontalRadiation[m * hoursInDay + h];
            }
        }
        return ret;
    }

} // namespace openstudio::isomodel