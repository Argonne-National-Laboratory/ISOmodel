#include "SolarRadiation.hpp"

namespace openstudio {
    namespace isomodel {

        static double SurfaceAzimuths[] = { 0, -PI / 4, -PI / 2, -3 * PI / 4, PI, 3 * PI / 4, PI / 2, PI / 4 };

        SolarRadiation::SolarRadiation(TimeFrame* frame, EpwData* wdata, double tilt)
            : m_frame(frame), m_epwData(wdata), m_groundReflectance(0.14)
        {
            m_surfaceTilt = tilt / 2.0;
            m_sinTilt = std::sin(m_surfaceTilt);
            m_cosTilt = std::cos(m_surfaceTilt);
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

            for (int i = 0; i < 8760; i++) {
                double rev = calculateRevolutionAngle(m_frame->YTD[i]);
                double eq = calculateEquationOfTime(rev);
                double ast = calculateApparentSolarTime(m_frame->Hour[i], eq);
                double dec = calculateSolarDeclination(rev);
                double sha = calculateSolarHourAngle(ast);
                double alt = calculateSolarAltitude(dec, sha);
                double sAz = calculateSolarAzimuth(calculateSolarAzimuthSin(dec, sha, alt), calculateSolarAzimuthCos(dec, sha, alt));

                double ground = calculateGroundReflectedIrradiance(vecEB[i], vecED[i], m_groundReflectance, alt, m_surfaceTilt);

                for (int s = 0; s < NUM_SURFACES; s++) {
                    double ssa = calculateSurfaceSolarAzimuth(sAz, SurfaceAzimuths[s]);
                    double inc = calculateAngleOfIncidence(alt, ssa, m_surfaceTilt);
                    double direct = calculateTotalDirectBeamIrradiance(vecEB[i], inc);
                    double diff = calculateTotalDiffuseIrradiance(vecED[i], calculateDiffuseAngleOfIncidenceFactor(inc), m_surfaceTilt);

                    m_eglobeFlat[i * NUM_SURFACES + s] = calculateTotalIrradiance(direct, diff, ground);
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