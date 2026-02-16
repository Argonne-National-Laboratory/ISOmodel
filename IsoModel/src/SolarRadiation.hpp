/*
 * SolarRadiation.hpp
 *
 * REFACTORING: PERFORMANCE & MEMORY OPTIMIZATION
 * 1. Memory: Flattened 2D vectors to 1D to reduce heap fragmentation and
 * allocation cost.
 * 2. Lazy Allocation: Statistical vectors are now allocated only when
 * Calculate(true) is called.
 * 3. Physics: Pre-calculation of daily solar geometry.
 * 4. Documentation: Includes equation references to ASHRAE 2013 and Duffie &
 * Beckman.
 */

#pragma once
#include "Constants.hpp"
#include "ISOModelAPI.hpp"
#include "TimeFrame.hpp"

#include <array>
#include <cmath>
#include <vector>

namespace openstudio::isomodel {

// Forward declaration
class EpwData;

class ISOMODEL_API SolarRadiation {
protected:
  TimeFrame *m_frame = nullptr;
  EpwData *m_epwData = nullptr;

  // In-class initialization
  double m_surfaceTilt = 0.0;
  double m_localMeridian = 0.0;
  double m_longitude = 0.0;
  double m_latitude = 0.0;
  double m_groundReflectance = DEFAULT_GROUND_REFLECTANCE;

  // Optimization: Pre-calculated Latitude Trig
  double m_sinLat = 0.0;
  double m_cosLat = 0.0;

  // Optimized Storage (Primary Output)
  std::vector<double> m_eglobeFlat;

  // Averages (Lazy Allocated - Empty by default)
  // Flattened 2D vectors to 1D for performance (Stride = NUM_VERTICAL_SURFACES or
  // 24 hours)
  std::vector<double> m_monthlyDryBulbTemp;
  std::vector<double> m_monthlyDewPointTemp;
  std::vector<double> m_monthlyRelativeHumidity;
  std::vector<double> m_monthlyWindspeed;
  std::vector<double> m_monthlyGlobalHorizontalRadiation;

  // Flattened: Index = month * numSurfaces + surface
  std::vector<double> m_monthlySolarRadiation;

  // Flattened: Index = month * 24 + hour
  std::vector<double> m_hourlyDryBulbTemp;
  std::vector<double> m_hourlyDewPointTemp;
  std::vector<double> m_hourlyGlobalHorizontalRadiation;

  // Performance caches
  double m_sinTilt = 0.0;
  double m_cosTilt = 0.0;
  std::array<double, NUM_VERTICAL_SURFACES> m_surfSin{};
  std::array<double, NUM_VERTICAL_SURFACES> m_surfCos{};

public:
  SolarRadiation(TimeFrame *frame, EpwData *wdata, double tilt = PI);

  ~SolarRadiation() = default;

  // set computeAverages=false to skip expensive stat allocations if not needed
  void Calculate(bool computeAverages = true);

  void calculateSurfaceSolarRadiation();
  void calculateAverages();
  void calculateMonthAvg(int midx, int cnt);
  void clearMonthlyAvg(int midx);

  // --- Inline Helpers (Math Logic Preserved) ---

  // Calculates the revolution angle in radians of the earth around the sun.
  // Ref: Duffie & Beckman Eq 1.4.2 (approx)
  [[nodiscard]] double calculateRevolutionAngle(int dayOfYear) const { return 2.0 * PI * dayOfYear / DAYS_IN_YEAR; }

  // Calculates the equation of time (EOT) in minutes.
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 1
  // Ref: Duffie & Beckman Eq 1.5.3 (Spencer 1971)
  [[nodiscard]] double calculateEquationOfTime(double B) const {
    return 2.2918 * (0.0075 + 0.1868 * std::cos(B) - 3.2077 * std::sin(B) -
                     1.4615 * std::cos(2 * B) - 4.089 * std::sin(2 * B));
  }

  // Calculates the apparent Solar Time (AST) in hours.
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 3
  // Ref: Duffie & Beckman Eq 1.5.2
  [[nodiscard]] double calculateApparentSolarTime(int localStandardTime, double equationOfTime) const {
    return localStandardTime + equationOfTime / 60.0 +
           (m_longitude - m_localMeridian) * (12.0 / PI);
  }

  // Calculates the solar declination (delta) in radians.
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 5
  // Ref: Duffie & Beckman Eq 1.6.1b (Spencer 1971)
  [[nodiscard]] double calculateSolarDeclination(double B) const {
    return 0.006918 - 0.399912 * std::cos(B) + 0.070257 * std::sin(B) -
           0.006758 * std::cos(2.0 * B) + 0.000907 * std::sin(2.0 * B) -
           0.002697 * std::cos(3.0 * B) + 0.00148 * std::sin(3.0 * B);
  }

  // Calculates the solar hour angle (H) in radians.
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 4
  // Ref: Duffie & Beckman Eq 1.6.4 (15 degrees per hour from solar noon)
  [[nodiscard]] double calculateSolarHourAngle(double ast) const { return (ast - 12) * (PI / 12.0); }

  // Calculates the solar altitude angle (beta) in radians.
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 6
  // Ref: Duffie & Beckman Eq 1.6.5
  [[nodiscard]] double calculateSolarAltitude(double dec, double sha) const {
    return std::asin(std::cos(m_latitude) * std::cos(dec) * std::cos(sha) +
                     std::sin(m_latitude) * std::sin(dec));
  }

  // Solar azimuth helpers
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 7
  // Ref: Duffie & Beckman Eq 1.6.6
  [[nodiscard]] double calculateSolarAzimuthSin(double dec, double H, double beta) const {
    return std::sin(H) * std::cos(dec) / std::cos(beta);
  }
  [[nodiscard]] double calculateSolarAzimuthCos(double dec, double H, double beta) const {
    return (std::cos(H) * std::cos(dec) * std::sin(m_latitude) -
            std::sin(dec) * std::cos(m_latitude)) /
           std::cos(beta);
  }
  [[nodiscard]] double calculateSolarAzimuth(double sina, double cosa) const { return std::atan2(sina, cosa); }

  // Ground reflected radiation (Isotropic Model)
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 23
  // Ref: Duffie & Beckman Eq 2.15.1
  [[nodiscard]] double calculateGroundReflectedIrradiance(double eb, double ed, double rho, double beta,
                                            double tilt) const {
    return (eb * std::sin(beta) + ed) * rho * (1 - std::cos(tilt)) / 2;
  }

  // Surface solar azimuth
  [[nodiscard]] double calculateSurfaceSolarAzimuth(double solAz, double surfAz) const {
    return std::fabs(solAz - surfAz);
  }

  // Angle of incidence (theta)
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 8
  // Ref: Duffie & Beckman Eq 1.6.2
  [[nodiscard]] double calculateAngleOfIncidence(double beta, double gamma, double tilt) const {
    return std::acos(std::cos(beta) * std::cos(gamma) * std::sin(tilt) +
                     std::sin(beta) * std::cos(tilt));
  }

  // Direct beam irradiance on surface
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 9
  [[nodiscard]] double calculateTotalDirectBeamIrradiance(double eb, double theta) const {
    return eb * std::max(std::cos(theta), 0.0);
  }

  // Diffuse angle of incidence factor (Y)
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 22
  [[nodiscard]] double calculateDiffuseAngleOfIncidenceFactor(double theta) const {
    return std::max(0.45, 0.55 + 0.437 * std::cos(theta) + 0.313 * std::pow(std::cos(theta), 2.0));
  }

  // Total diffuse irradiance (Surface Diffuse)
  // Ref: ASHRAE Fundamentals 2013, Ch 14, Eq 21 & 22
  [[nodiscard]] double calculateTotalDiffuseIrradiance(double ed, double Y, double tilt) const {
    return (tilt > PI / 2) ? ed * Y * std::sin(tilt) : ed * (Y * std::sin(tilt) + std::cos(tilt));
  }

  // Total irradiance (Global Surface)
  [[nodiscard]] double calculateTotalIrradiance(double dir, double diff, double ground) const {
    return dir + diff + ground;
  }

  // --- Getters with Legacy Interface Support ---

  [[nodiscard]] double surfaceTilt() const { return m_surfaceTilt; }
  [[nodiscard]] double localMeridian() const { return m_localMeridian; }
  [[nodiscard]] double lon() const { return m_longitude; }
  [[nodiscard]] double lat() const { return m_latitude; }
  [[nodiscard]] double groundReflectance() const { return m_groundReflectance; }

  // Reconstructs 2D vector on demand for legacy support
  [[nodiscard]] std::vector<std::vector<double>> eglobe() const;

  [[nodiscard]] const std::vector<double> &eglobeFlat() const { return m_eglobeFlat; }

  [[nodiscard]] std::vector<double> monthlyDryBulbTemp() const { return m_monthlyDryBulbTemp; }
  [[nodiscard]] std::vector<double> monthlyDewPointTemp() const { return m_monthlyDewPointTemp; }
  [[nodiscard]] std::vector<double> monthlyRelativeHumidity() const { return m_monthlyRelativeHumidity; }
  [[nodiscard]] std::vector<double> monthlyWindspeed() const { return m_monthlyWindspeed; }
  [[nodiscard]] std::vector<double> monthlyGlobalHorizontalRadiation() const {
    return m_monthlyGlobalHorizontalRadiation;
  }

  // Legacy getters that reconstruct 2D vectors from flat storage
  [[nodiscard]] std::vector<std::vector<double>> monthlySolarRadiation() const;
  [[nodiscard]] std::vector<std::vector<double>> hourlyDryBulbTemp() const;
  [[nodiscard]] std::vector<std::vector<double>> hourlyDewPointTemp() const;
  [[nodiscard]] std::vector<std::vector<double>> hourlyGlobalHorizontalRadiation() const;
};

} // namespace openstudio::isomodel
