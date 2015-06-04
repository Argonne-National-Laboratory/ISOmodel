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

private:
  double m_powerDensityOccupied;
  double m_powerDensityUnoccupied;
  double m_dimmingFraction;
  double m_exteriorEnergy;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_LIGHTING_HPP
