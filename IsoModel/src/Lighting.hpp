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
#ifndef ISOMODEL_LIGHTING_HPP
#define ISOMODEL_LIGHTING_HPP

namespace openstudio {
namespace isomodel {
class Lighting
{
public:
  Lighting(void);
  ~Lighting(void);

  /**
  * Lighting power density occupied (W/m2).
  */
  double powerDensityOccupied() const {
    return m_powerDensityOccupied;
  }

  void setPowerDensityOccupied(double value) {
    m_powerDensityOccupied = value;
  }

  /**
  * Lighting power density unoccupied (W/m2).
  */
  double powerDensityUnoccupied() const {
    return m_powerDensityUnoccupied;
  }

  void setPowerDensityUnoccupied(double value) {
    m_powerDensityUnoccupied = value;
  }

  /**
  * Daylight sensor dimming fraction (unitless).
  * Illum controls are set to 1 if there is no control.
  * See iso 15193 Annex F/G for values.
  */
  double dimmingFraction() const {
    return m_dimmingFraction;
  }

  void setDimmingFraction(double value) {
    m_dimmingFraction = value;
  }

  /**
  * Exterior lighting power (W).
  */
  double exteriorEnergy() const {
    return m_exteriorEnergy;
  }

  void setExteriorEnergy(double value) {
    m_exteriorEnergy = value;
  }

  /**
  * Sunrise (24-hour time). Defaults to 7.0.
  */
  double n_day_start() {
    return m_n_day_start;
  }

  void set_n_day_start(double n_day_start) {
    m_n_day_start = n_day_start;
  }

  /**
  * Sunset (24-hour time). Defaults to 18.0.
  */
  double n_day_end() {
    return m_n_day_end;
  }

  void set_n_day_end(double n_day_end) {
    m_n_day_end = n_day_end;
  }

  /**
  * Number of occupied weeks for lighting purposes.
  */
  double n_weeks() {
    return m_n_weeks;
  }

  void set_n_weeks(double n_weeks) {
    m_n_weeks = n_weeks;
  }

  /**
  * Electric internal gains. XXX: This appears to be a ratio/factor but I'm not sure - BAA@2015-06-08.
  */
  double elecInternalGains() {
    return m_elecInternalGains;
  }

  void setElecInternalGains(double electInternalGains) {
    m_elecInternalGains = electInternalGains;
  }

  /**
  * Permanent lighting power density (W/m2). Lighting that is always on regardless of occupancy
  * (e.g. emergency lights).
  */

  double permLightPowerDensity() {
    return m_permLightPowerDensity;
  }

  void setPermLightPowerDensity(double permLightPowerDensity) {
    m_permLightPowerDensity = permLightPowerDensity;
  }

private:
  double m_powerDensityOccupied;
  double m_powerDensityUnoccupied;
  double m_dimmingFraction;
  double m_exteriorEnergy;
  // Members with default values:
  double m_n_day_start = 7.0; // TODO: sunrise shouldn't be the same every month.
  double m_n_day_end = 18.0; // TODO: sunrise shouldbe be the same every month.
  double m_n_weeks = 50.0;
  double m_elecInternalGains = 1.0;
  double m_permLightPowerDensity = 0.0;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_LIGHTING_HPP
