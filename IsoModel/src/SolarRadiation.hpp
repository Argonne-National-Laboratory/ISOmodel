/*
 * SolarRadiation.hpp
 *
 * OPTIMIZATION SUMMARY:
 * 1. Circular Dependency Fix: Removed `#include "EpwData.hpp"` to resolve the
 * "too many include files" / circular dependency error. The class uses a
 * pointer to EpwData, so the forward declaration is sufficient here.
 * 2. Interface Stability: The math helper functions (calculateRevolutionAngle, etc.)
 * are preserved with their original ASHRAE citations. While the optimized .cpp
 * implementation bypasses some of these for speed (using vector algebra),
 * these functions remain available for other potential uses or verification.
 */

#ifndef ISOMODEL_SOLAR_RADIATION_HPP
#define ISOMODEL_SOLAR_RADIATION_HPP

#include "ISOModelAPI.hpp"
#include <cmath>
#include <vector>
#include "TimeFrame.hpp"
 // REMOVED: #include "EpwData.hpp" to fix circular dependency

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

            // Calculates the revolution angle in radians of the earth around the sun.
            // ASHRAE2013 Fundamentals, Ch. 14, eq. 6. with dayOfYear going from 0 to 364 not 1 to 365
            // Beckman and Duffie 1.4.2  (use B&D's notation of B to replace capGamma of ASHRAE = revolution angle)
            double calculateRevolutionAngle(int dayOfYear) { return 2.0 * PI * dayOfYear / 365.0; }

            // Calculates the difference between the apparent solar time and mean solar time (the equation of time).
            // ASHRAE2013 Fundamentals, Ch. 14, eq. 5., Beckmand and Duffie 1.4.2 who use B for revolution angle
            double calculateEquationOfTime(double B) {
                return 2.2918 * (0.0075 + 0.1868 * cos(B) - 3.2077 * sin(B)
                    - 1.4615 * cos(2 * B) - 4.089 * sin(2 * B));
            }

            /**
            * Calculates the apparent Solar Time in hours.
            * ASHRAE2013 Fundamentals, Ch. 14, eq. 7.
            * Note that because we use radians for longitude, we divide by 15*pi/180 instead of 15, or pi / 12.
            */
            double calculateApparentSolarTime(int localStandardTime, double equationOfTime) {
                return localStandardTime + equationOfTime / 60.0 + (m_longitude - m_localMeridian) / (PI / 12.0);
            }

            /**
            * Calculates the solar declination in radians. The following is a more accurate formula
            * for declination as taken from "Solar Engineering of Thermal Processes,"
            * Duffie and Beckman p. 14, eq. 1.6.1b.  B is the revolution enagle
            * cut this off at cost 2B and sin 2B if trimming microseconds is important
            */
            double calculateSolarDeclination(double B) {
                return 0.006918 - 0.399912 * cos(B) + 0.070257 * sin(B)
                    - 0.006758 * cos(2.0 * B) + 0.000907 * sin(2.0 * B)
                    - 0.002697 * cos(3.0 * B) + 0.00148 * sin(3.0 * B);
            }

            // Calculates the solar hour angle in radians from ASHRAE2013 Fundamentals, Ch. 14, eq. 11.
            double calculateSolarHourAngle(double ast) { return (ast - 12) * 15 * PI / 180.0; }

            // Calculates the solar altitude angle in radians.
            // ASHRAE2013 Fundamentals, Ch. 14, eq. 12.
            double calculateSolarAltitude(double dec, double sha) {
                return asin(cos(m_latitude) * cos(dec) * cos(sha) + sin(m_latitude) * sin(dec));
            }

            // Calculates the sin and cos of the solar azimuth to get solar azimuth.
            // from ASHRAE2025 Fundamentals, Ch. 14, eq. 14 and 15  H = hour angle, dec = declination and beta = solar altitude
            // now we get the solar azimuth as from tan(solar azimuth) = sin (solar azimuth)/cos(solar azimuth)
            double calculateSolarAzimuthSin(double dec, double H, double beta) { return sin(H) * cos(dec) / cos(beta); }
            double calculateSolarAzimuthCos(double dec, double H, double beta) {
                return (cos(H) * cos(dec) * sin(m_latitude) - sin(dec) * cos(m_latitude)) / cos(beta);
            }
            double calculateSolarAzimuth(double sina, double cosa) { return atan2(sina, cosa); }

            // Calculate the total ground reflected radiation.
            // ASHRAE2025 Fundamentals, Ch. 14, eq. 30.
            // Eb = normal beam irradiance, Ed diffuce irrandiance, rho = reflectance, beta = solar altitude, tilt  = surface tilt angle
            double calculateGroundReflectedIrradiance(double eb, double ed, double rho, double beta, double tilt) {
                return (eb * sin(beta) + ed) * rho * (1 - cos(tilt)) / 2;
            }

            // Calculates the surface solar azimuth(the difference between the surface and solar azimuths).
            // solarAzimuth and surfaceAzimuth should be in radians.Result in radians.
            // ASHRAE2013/2017/2025 Fundamentals, Ch. 14, eq. 21 
            // note from RTM - why fabs??? Thats not in ashrae??
            double calculateSurfaceSolarAzimuth(double solAz, double surfAz) { return fabs(solAz - surfAz); }

            // Calculates the angle of incidence of the sun on the surface.
            // ASHRAE2013/2017/2025 Fundamentals, Ch. 14, eq. 22.
            // beta = solar altitude, gamma = surface-solar-azimush, 
            double calculateAngleOfIncidence(double beta, double gamma, double tilt) {
                return acos(cos(beta) * cos(gamma) * sin(tilt) + sin(beta) * cos(tilt));
            }

            // Calculates the total direct beam irradiance on a surface.
            // ASHRAE2013/2017/2025 Fundamentals, Ch. 14, eq. 26., theta = angle of incidence
            double calculateTotalDirectBeamIrradiance(double eb, double theta) { return eb * std::max(cos(theta), 0.0); }

            // Calculates the ratio of clear - sky diffuse irradiance on a vertical surface to that on a horizontal surface.
            // ASHRAE2013 Fundamentals, Ch. 14, eq. 28, theta = angle of incidence
            double calculateDiffuseAngleOfIncidenceFactor(double theta) {
                return std::max(0.45, 0.55 + 0.437 * cos(theta) + 0.313 * std::pow(cos(theta), 2.0));
            }

            // Calculates the total diffuse irradiance on the surface.
            // ASHRAE2013 Fundamentals, Ch. 14, eq. 29, 30.
            // ed = diffuse horizontal irrandiance, tilt - surface tilt angle, Y = diffuse angle of incidence factor
            double calculateTotalDiffuseIrradiance(double ed, double Y, double tilt) {
                return (tilt > PI / 2) ? ed * Y * sin(tilt) : ed * (Y * sin(tilt) + cos(tilt));
            }
            // Calculates the total irradiance reaching a surface.
            // ASHRAE2013 Fundamentals, Ch. 14, eq. 25.
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