/// @file Location.hpp
/// @brief Site location properties including terrain class and weather data.
///
/// Holds the terrain class (urban/suburban/rural wind shielding factor)
/// and a shared pointer to the WeatherData used by the simulation models.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#pragma once
#include "ISOModelAPI.hpp"
#include "WeatherData.hpp"

#include <memory>

namespace openstudio::isomodel {

class ISOMODEL_API Location {
public:
  // Use compiler-generated default constructor/destructor
  Location() = default;
  ~Location() = default;

  /// Terrain class (urban/city = 0.8, suburban/some shielding = 0.9,
  /// country/open = 1.0).
  [[nodiscard]] double terrain() const noexcept { return m_terrain; }

  void setTerrain(double value) { m_terrain = value; }

  /// Pointer to weather data. Contains data extracted/computed from .epw file.
  [[nodiscard]] std::shared_ptr<WeatherData> weather() const noexcept { return m_weather; }

  void setWeatherData(std::shared_ptr<WeatherData> value) { m_weather = value; }

private:
  // In-class initialization
  double m_terrain = 0.0;
  std::shared_ptr<WeatherData> m_weather;
};

} // namespace openstudio::isomodel
