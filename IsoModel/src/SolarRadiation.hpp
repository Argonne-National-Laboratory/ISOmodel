/*
 * SolarRadiation.hpp
 *
 * REFACTORING:
 * 1. Modern C++: Used constexpr for constants and in-class initialization for members.
 * 2. Cleanup: Defaulted destructor and removed redundant includes.
 * 3. ABI: Public interface remains strictly identical.
 */

#ifndef ISOMODEL_SOLAR_RADIATION_HPP
#define ISOMODEL_SOLAR_RADIATION_HPP

#include "Constants.hpp"
#include "ISOModelAPI.hpp"
#include "TimeFrame.hpp"
#include <cmath>
#include <vector>

namespace openstudio::isomodel {

    // Forward declaration
    class EpwData;


    class ISOMODEL_API SolarRadiation {
    protected:
        TimeFrame* m_frame = nullptr;
        EpwData* m_epwData = nullptr;

        // In-class initialization
        double m_surfaceTilt = 0.0;
        double m_localMeridian = 0.0;
        double m_longitude = 0.0;
        double m_latitude = 0.0;
        double m_groundReflectance = defaultGroundReflectance;

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
        double m_sinTilt = 0.0;
        double m_cosTilt = 0.0;
        double m_surfSin[numVerticalSurfaces] = {}; // Zero-initialized
        double m_surfCos[numVerticalSurfaces] = {};

    public:
        SolarRadiation(TimeFrame* frame, EpwData* wdata, double tilt = PI);
        
        // Destructor defaulted in header
        ~SolarRadiation() = default;

        void Calculate();
        void calculateSurfaceSolarRadiation();
        void calculateAverages();
        void calculateMonthAvg(int midx, int cnt);
        void clearMonthlyAvg(int midx);
        
        // --- Inline Helpers (Math Logic Preserved) ---

        // Calculates the revolution angle in radians of the earth around the sun.
        double calculateRevolutionAngle(int dayOfYear) { 
            return 2.0 * PI * dayOfYear / 365.0; 
        }
        
        // Calculates the equation of time.
        double calculateEquationOfTime(double B) {
            return 2.2918 * (0.0075 + 0.1868 * std::cos(B) - 3.2077 * std::sin(B) 
                - 1.4615 * std::cos(2 * B) - 4.089 * std::sin(2 * B));
        }

        // Calculates the apparent Solar Time in hoursInDay.
        double calculateApparentSolarTime(int localStandardTime, double equationOfTime) {
            return localStandardTime + equationOfTime / 60.0 + (m_longitude - m_localMeridian) / (PI / 12.0);
        }

        // Calculates the solar declination in radians.
        double calculateSolarDeclination(double B) {
            return 0.006918 - 0.399912 * std::cos(B) + 0.070257 * std::sin(B)
                - 0.006758 * std::cos(2.0 * B) + 0.000907 * std::sin(2.0 * B)
                - 0.002697 * std::cos(3.0 * B) + 0.00148 * std::sin(3.0 * B);
        }

        // Calculates the solar hour angle in radians.
        double calculateSolarHourAngle(double ast) { 
            return (ast - 12) * 15 * PI / 180.0; 
        }

        // Calculates the solar altitude angle in radians.
        double calculateSolarAltitude(double dec, double sha) {
            return std::asin(std::cos(m_latitude) * std::cos(dec) * std::cos(sha) + std::sin(m_latitude) * std::sin(dec));
        }

        // Solar azimuth helpers
        double calculateSolarAzimuthSin(double dec, double H, double beta) { 
            return std::sin(H) * std::cos(dec) / std::cos(beta); 
        }
        double calculateSolarAzimuthCos(double dec, double H, double beta) {
            return (std::cos(H) * std::cos(dec) * std::sin(m_latitude) - std::sin(dec) * std::cos(m_latitude)) / std::cos(beta);
        }
        double calculateSolarAzimuth(double sina, double cosa) { 
            return std::atan2(sina, cosa); 
        }

        // Ground reflected radiation
        double calculateGroundReflectedIrradiance(double eb, double ed, double rho, double beta, double tilt) {
            return (eb * std::sin(beta) + ed) * rho * (1 - std::cos(tilt)) / 2;
        }

        // Surface solar azimuth
        double calculateSurfaceSolarAzimuth(double solAz, double surfAz) { 
            return std::fabs(solAz - surfAz); 
        }

        // Angle of incidence
        double calculateAngleOfIncidence(double beta, double gamma, double tilt) {
            return std::acos(std::cos(beta) * std::cos(gamma) * std::sin(tilt) + std::sin(beta) * std::cos(tilt));
        }

        // Direct beam irradiance
        double calculateTotalDirectBeamIrradiance(double eb, double theta) { 
            return eb * std::max(std::cos(theta), 0.0); 
        }

        // Diffuse angle factor
        double calculateDiffuseAngleOfIncidenceFactor(double theta) {
            return std::max(0.45, 0.55 + 0.437 * std::cos(theta) + 0.313 * std::pow(std::cos(theta), 2.0));
        }

        // Total diffuse irradiance
        double calculateTotalDiffuseIrradiance(double ed, double Y, double tilt) {
            return (tilt > PI / 2) ? ed * Y * std::sin(tilt) : ed * (Y * std::sin(tilt) + std::cos(tilt));
        }

        // Total irradiance
        double calculateTotalIrradiance(double dir, double diff, double ground) { 
            return dir + diff + ground; 
        }

        // --- Getters ---
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

} // namespace openstudio::isomodel
#endif