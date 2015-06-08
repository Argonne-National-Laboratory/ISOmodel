/**********************************************************************
 *  Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 *  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/
#ifndef ISOMODEL_LOCATION_HPP
#define ISOMODEL_LOCATION_HPP

#include "WeatherData.hpp"

namespace openstudio {
namespace isomodel {

class Location
{
public:
  Location(void);
  ~Location(void);

  /**
  * Terrain class (urban/city = 0.8, suburban/some shielding = 0.9, country/open = 1.0).
  */
  double terrain() const {
    return m_terrain;
  }
  
  void setTerrain(double value) {
    m_terrain = value;
  }

  /**
  * Pointer to weather data. Contains data extracted/computed from .epw file.
  */
  std::shared_ptr<WeatherData> weather() const {
    return m_weather;
  }

  void setWeatherData(std::shared_ptr<WeatherData> value) {
    m_weather = value;
  }

private:
  double m_terrain;
  std::shared_ptr<WeatherData> m_weather;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_LOCATION_HPP

