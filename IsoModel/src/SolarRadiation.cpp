/*
 * SolarRadiation.cpp
 *
 * OPTIMIZATION SUMMARY:
 * 1. Day-Based Geometry: Orbital parameters (declination, equation of time) depend
 * only on the day of the year. Recalculated only when the day changes (365 times)
 * instead of every hour (8760 times).
 * 2. Trig Removal: Replaced expensive trigonometric calls (asin, acos, atan2) in
 * the inner surface loop with vector algebra identities.
 * - Replaced 'calculateAngleOfIncidence' (ASHRAE eq 22) with dot product logic.
 * - Replaced 'calculateSolarAzimuth' components with direct sin/cos derivation.
 */

#include "SolarRadiation.hpp"
#include "EpwData.hpp"

namespace openstudio {
    namespace isomodel {
        /**
         * Surface Azimuths of the "building" to calculate solar radiation for, in radians.
         * In the order S, SE, E, NE, N, NW, W, SW.
         * In degrees, the azimuths are: 0, -45, -90, -135, 180, 135, 90, 45.
         */
        static double SurfaceAzimuths[] = { 0, -PI / 4, -PI / 2, -3 * PI / 4, PI, 3 * PI / 4, PI / 2, PI / 4 };

        SolarRadiation::SolarRadiation(TimeFrame* frame, EpwData* wdata, double tilt)
            : m_frame(frame), m_epwData(wdata), m_groundReflectance(0.14)
        {
            m_surfaceTilt = tilt / 2.0;
            m_sinTilt = std::sin(m_surfaceTilt);
            m_cosTilt = std::cos(m_surfaceTilt);
            // don't forget to convert from degrees to radians so * by PI/180
            m_longitude = wdata->longitude() * (PI / 180.0);
            m_localMeridian = wdata->timezone() * 15.0 * (PI / 180.0);
            m_latitude = wdata->latitude() * (PI / 180.0);

            for (int i = 0; i < NUM_SURFACES; ++i) {
                m_surfSin[i] = std::sin(SurfaceAzimuths[i]);
                m_surfCos[i] = std::cos(SurfaceAzimuths[i]);
            }

            m_eglobeFlat.assign(8760 * NUM_SURFACES, 0.0);
            m_monthlyDryBulbTemp.assign(MONTHS, 0.0);
            m_monthlyDewPointTemp.assign(MONTHS, 0.0);
            m_monthlyRelativeHumidity.assign(MONTHS, 0.0);
            m_monthlyWindspeed.assign(MONTHS, 0.0);
            m_monthlyGlobalHorizontalRadiation.assign(MONTHS, 0.0);

            m_monthlySolarRadiation.assign(MONTHS, std::vector<double>(NUM_SURFACES, 0.0));
            m_hourlyDryBulbTemp.assign(MONTHS, std::vector<double>(HOURS, 0.0));
            m_hourlyDewPointTemp.assign(MONTHS, std::vector<double>(HOURS, 0.0));
            m_hourlyGlobalHorizontalRadiation.assign(MONTHS, std::vector<double>(HOURS, 0.0));
        }

        SolarRadiation::~SolarRadiation() {}

        std::vector<std::vector<double>> SolarRadiation::eglobe() {
            std::vector<std::vector<double>> legacy(8760, std::vector<double>(NUM_SURFACES, 0.0));
            for (int i = 0; i < 8760; ++i) {
                for (int s = 0; s < NUM_SURFACES; ++s) {
                    legacy[i][s] = m_eglobeFlat[i * NUM_SURFACES + s];
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

            // Caching variables for day-based optimization
            int lastDay = -1;
            double eq = 0.0, dec = 0.0;
            double sinDec = 0.0, cosDec = 0.0;

            for (int i = 0; i < 8760; i++) {

                // OPTIMIZATION: Recalculate daily variables only when day changes
                // ASHRAE2013 Fundamentals, Ch. 14, eq. 5, 6, 1.6.1b
                int currentDay = m_frame->YTD[i];
                if (currentDay != lastDay) {
                    double rev = calculateRevolutionAngle(currentDay);
                    eq = calculateEquationOfTime(rev);
                    dec = calculateSolarDeclination(rev);
                    sinDec = std::sin(dec);
                    cosDec = std::cos(dec);
                    lastDay = currentDay;
                }

                // ASHRAE2013 Fundamentals, Ch. 14, eq. 7. (Apparent Solar Time)
                double ast = calculateApparentSolarTime(m_frame->Hour[i], eq);
                // ASHRAE2013 Fundamentals, Ch. 14, eq. 11. (Hour Angle)
                double sha = calculateSolarHourAngle(ast);
                double cosSha = std::cos(sha);
                double sinSha = std::sin(sha);

                // OPTIMIZATION: TRIG REMOVAL
                // Replaced 'calculateSolarAltitude' (ASHRAE eq. 12) 
                // We compute sinBeta (sin(Altitude)) directly via dot product
                double sinBeta = std::cos(m_latitude) * cosDec * cosSha + std::sin(m_latitude) * sinDec;

                // cosBeta = sqrt(1 - sinBeta^2). Clamp to avoid sqrt(negative).
                // If sun is below horizon, sinBeta < 0.
                double cosBeta;
                if (sinBeta >= 1.0) cosBeta = 0.0;
                else if (sinBeta <= -1.0) cosBeta = 0.0;
                else cosBeta = std::sqrt(1.0 - sinBeta * sinBeta);

                if (cosBeta < 1e-6) cosBeta = 1e-6; // Avoid division by zero

                // Solar Azimuth Sine/Cosine (Derived from ASHRAE eq 14, 15)
                double sinAz = sinSha * cosDec / cosBeta;
                double cosAz = (cosSha * cosDec * std::sin(m_latitude) - sinDec * std::cos(m_latitude)) / cosBeta;

                // OPTIMIZATION: Vectorized Ground Reflection
                // ASHRAE2025 Fundamentals, Ch. 14, eq. 30 (Ground Reflected)
                double ground = (vecEB[i] * sinBeta + vecED[i]) * m_groundReflectance * (1 - m_cosTilt) * 0.5;

                for (int s = 0; s < NUM_SURFACES; s++) {
                    // OPTIMIZATION: TRIG REMOVAL for Incidence Angle
                    // Replaces 'calculateAngleOfIncidence' (ASHRAE Eq. 22)
                    // cos(theta) = cosBeta * cos(Gamma) * sinTilt + sinBeta * cosTilt
                    // where Gamma = SolarAz - SurfaceAz
                    // cos(Gamma) = cos(SolarAz - SurfAz) = cosSol * cosSurf + sinSol * sinSurf

                    double cosGamma = cosAz * m_surfCos[s] + sinAz * m_surfSin[s];
                    double cosTheta = cosBeta * cosGamma * m_sinTilt + sinBeta * m_cosTilt;

                    // Direct Beam: ASHRAE Eq. 26
                    double direct = (cosTheta > 0.0) ? vecEB[i] * cosTheta : 0.0;

                    // Diffuse Angle of Incidence Factor: ASHRAE Eq. 28
                    // We clamp cosTheta for this empirical formula
                    double Y = std::max(0.45, 0.55 + 0.437 * cosTheta + 0.313 * cosTheta * cosTheta);

                    // Total Diffuse: ASHRAE Eq. 29, 30
                    double diff = (m_surfaceTilt > PI / 2) ?
                        vecED[i] * Y * m_sinTilt :
                        vecED[i] * (Y * m_sinTilt + m_cosTilt);

                    // Total Irradiance: ASHRAE Eq. 25
                    m_eglobeFlat[i * NUM_SURFACES + s] = direct + diff + ground;
                }
            }
        }

        void SolarRadiation::calculateAverages() {
            const auto& dataMap = m_epwData->data();
            int month = 0, midx = -1, cnt = 0;

            for (int i = 0; i < 8760; i++, cnt++) {
                if (m_frame->Month[i] != month) {
                    if (midx >= 0) calculateMonthAvg(midx, cnt);
                    month = m_frame->Month[i];
                    midx++;
                    clearMonthlyAvg(midx);
                    cnt = 0;
                }
                m_monthlyDryBulbTemp[midx] += dataMap[DBT][i];
                m_monthlyDewPointTemp[midx] += dataMap[DPT][i];
                m_monthlyRelativeHumidity[midx] += dataMap[RH][i];
                m_monthlyWindspeed[midx] += dataMap[WSPD][i];
                m_monthlyGlobalHorizontalRadiation[midx] += dataMap[EGH][i];

                int h = m_frame->Hour[i];
                m_hourlyDryBulbTemp[midx][h] += dataMap[DBT][i];
                m_hourlyDewPointTemp[midx][h] += dataMap[DPT][i];
                m_hourlyGlobalHorizontalRadiation[midx][h] += dataMap[EGH][i];

                for (int s = 0; s < NUM_SURFACES; s++) m_monthlySolarRadiation[midx][s] += m_eglobeFlat[i * NUM_SURFACES + s];
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
            for (int s = 0; s < NUM_SURFACES; s++) m_monthlySolarRadiation[midx][s] *= inv;

            double days = m_frame->monthLength(midx + 1);
            for (int h = 0; h < 24; h++) {
                m_hourlyDryBulbTemp[midx][h] /= days;
                m_hourlyDewPointTemp[midx][h] /= days;
                m_hourlyGlobalHorizontalRadiation[midx][h] /= days;
            }
        }

        void SolarRadiation::clearMonthlyAvg(int midx) {
            // Logic handled by constructor/initializers
        }

    } // namespace isomodel
} // namespace openstudio