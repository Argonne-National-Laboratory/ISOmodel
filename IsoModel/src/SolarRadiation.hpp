#ifndef ISOMODEL_SOLAR_RADIATION_HPP
#define ISOMODEL_SOLAR_RADIATION_HPP

#include "ISOModelAPI.hpp"
#include <cmath>
#include <vector>
#include "TimeFrame.hpp"
#include "EpwData.hpp"

namespace openstudio {
    namespace isomodel {

        // Forward declaration to fix "identifier EpwData" errors
        class EpwData;

        const double PI = 3.14159265358979323846;
        const int NUM_SURFACES = 8;
        const int MONTHS = 12;
        const int HOURS = 24;

        class ISOMODEL_API SolarRadiation {
        protected:
            openstudio::isomodel::TimeFrame* m_frame;
            openstudio::isomodel::EpwData* m_epwData;

            double m_surfaceTilt;
            double m_localMeridian;
            double m_longitude;
            double m_latitude;
            double m_groundReflectance;

            // Optimized Storage
            std::vector<double> m_eglobeFlat;

            // Averages
            std::vector<double> m_monthlyDryBulbTemp;
            std::vector<double> m_monthlyDewPointTemp;
            std::vector<double> m_monthlyRelativeHumidity;
            std::vector<double> m_monthlyWindspeed;
            std::vector<double> m_monthlyGlobalHorizontalRadiation;
            std::vector<std::vector<double>> m_monthlySolarRadiation;
            std::vector<std::vector<double>> m_hourlyDryBulbTemp;
            std::vector<std::vector<double>> m_hourlyDewPointTemp;
            std::vector<std::vector<double>> m_hourlyGlobalHorizontalRadiation;

            // Performance caches
            double m_sinTilt, m_cosTilt;
            double m_surfSin[NUM_SURFACES];
            double m_surfCos[NUM_SURFACES];

        public:
            SolarRadiation(TimeFrame* frame, EpwData* wdata, double tilt = PI);
            ~SolarRadiation();

            void Calculate();
            void calculateSurfaceSolarRadiation();
            void calculateAverages();
            void calculateMonthAvg(int midx, int cnt);
            void clearMonthlyAvg(int midx);

            // --- Restored Original Math Functions for Compatibility ---
            double calculateRevolutionAngle(int dayOfYear) { return 2.0 * PI * dayOfYear / 365.0; }
            double calculateEquationOfTime(double rev) {
                return 2.2918 * (0.0075 + 0.1868 * cos(rev) - 3.2077 * sin(rev) - 1.4615 * cos(2 * rev) - 4.089 * sin(2 * rev));
            }
            double calculateApparentSolarTime(int hr, double eq) {
                return hr + eq / 60.0 + (m_longitude - m_localMeridian) / (PI / 12.0);
            }
            double calculateSolarDeclination(double rev) {
                return 0.006918 - 0.399912 * cos(rev) + 0.070257 * sin(rev) - 0.006758 * cos(2.0 * rev) + 0.000907 * sin(2.0 * rev);
            }
            double calculateSolarHourAngle(double ast) { return 15 * (ast - 12) * PI / 180.0; }
            double calculateSolarAltitude(double dec, double sha) {
                return asin(cos(m_latitude) * cos(dec) * cos(sha) + sin(m_latitude) * sin(dec));
            }
            double calculateSolarAzimuthSin(double dec, double sha, double alt) { return sin(sha) * cos(dec) / cos(alt); }
            double calculateSolarAzimuthCos(double dec, double sha, double alt) {
                return (cos(sha) * cos(dec) * sin(m_latitude) - sin(dec) * cos(m_latitude)) / cos(alt);
            }
            double calculateSolarAzimuth(double s, double c) { return atan2(s, c); }
            double calculateGroundReflectedIrradiance(double eb, double ed, double rho, double alt, double tilt) {
                return (eb * sin(alt) + ed) * rho * (1 - cos(tilt)) / 2;
            }
            double calculateSurfaceSolarAzimuth(double solAz, double surfAz) { return fabs(solAz - surfAz); }
            double calculateAngleOfIncidence(double alt, double ssa, double tilt) {
                return acos(cos(alt) * cos(ssa) * sin(tilt) + sin(alt) * cos(tilt));
            }
            double calculateTotalDirectBeamIrradiance(double eb, double inc) { return eb * std::max(cos(inc), 0.0); }
            double calculateDiffuseAngleOfIncidenceFactor(double inc) {
                return std::max(0.45, 0.55 + 0.437 * cos(inc) + 0.313 * std::pow(cos(inc), 2.0));
            }
            double calculateTotalDiffuseIrradiance(double ed, double factor, double tilt) {
                return (tilt > PI / 2) ? ed * factor * sin(tilt) : ed * (factor * sin(tilt) + cos(tilt));
            }
            double calculateTotalIrradiance(double dir, double diff, double ground) { return dir + diff + ground; }

            // --- Original Getters ---
            double surfaceTilt() { return m_surfaceTilt; }
            double localMeridian() { return m_localMeridian; }
            double lon() { return m_longitude; }
            double lat() { return m_latitude; }
            double groundReflectance() { return m_groundReflectance; }

            std::vector<std::vector<double>> eglobe();
            const std::vector<double>& eglobeFlat() const { return m_eglobeFlat; }

            std::vector<double> monthlyDryBulbTemp() { return m_monthlyDryBulbTemp; }
            std::vector<double> monthlyDewPointTemp() { return m_monthlyDewPointTemp; }
            std::vector<double> monthlyRelativeHumidity() { return m_monthlyRelativeHumidity; }
            std::vector<double> monthlyWindspeed() { return m_monthlyWindspeed; }
            std::vector<double> monthlyGlobalHorizontalRadiation() { return m_monthlyGlobalHorizontalRadiation; }
            std::vector<std::vector<double>> monthlySolarRadiation() { return m_monthlySolarRadiation; }
            std::vector<std::vector<double>> hourlyDryBulbTemp() { return m_hourlyDryBulbTemp; }
            std::vector<std::vector<double>> hourlyDewPointTemp() { return m_hourlyDewPointTemp; }
            std::vector<std::vector<double>> hourlyGlobalHorizontalRadiation() { return m_hourlyGlobalHorizontalRadiation; }
        };

    } // namespace isomodel
} // namespace openstudio
#endif