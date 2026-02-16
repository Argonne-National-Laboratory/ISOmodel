#ifndef ISOMODEL_LOCATION_HPP
#define ISOMODEL_LOCATION_HPP

#include "ISOModelAPI.hpp"
#include "WeatherData.hpp"

#include <memory>

namespace openstudio::isomodel {

class ISOMODEL_API Location {
public:
  // Use compiler-generated default constructor/destructor
  Location() = default;
  ~Location() = default;

  /**
   * Terrain class (urban/city = 0.8, suburban/some shielding = 0.9,
   * country/open = 1.0).
   */
  double terrain() const { return m_terrain; }

  void setTerrain(double value) { m_terrain = value; }

  /**
   * Pointer to weather data. Contains data extracted/computed from .epw file.
   */
  std::shared_ptr<WeatherData> weather() const { return m_weather; }

  void setWeatherData(std::shared_ptr<WeatherData> value) { m_weather = value; }

private:
  // In-class initialization
  double m_terrain = 0.0;
  std::shared_ptr<WeatherData> m_weather;
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_LOCATION_HPP