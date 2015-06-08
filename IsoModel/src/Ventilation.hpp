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
#ifndef ISOMODEL_VENTILATION_HPP
#define ISOMODEL_VENTILATION_HPP

namespace openstudio {
namespace isomodel {
class Ventilation
{
public:
  Ventilation(void);
  ~Ventilation(void);

  /**
  * Ventilation intake rate occupied (L/s). Use 10 L/s/person as a default.
  */
  double supplyRate() const {
    return m_supplyRate;
  }

  void setSupplyRate(double value) {
    m_supplyRate = value;
  }

  /**
  * Ventilation exhaust rate occupied (L/s).
  */
  double supplyDifference() const {
    return m_supplyDifference;
  }

  void setSupplyDifference(double value) {
    m_supplyDifference = value;
  }

  /**
  * Efficiency of heat recovery (unitless. Use 0.0 for no heat recovery).
  */
  double heatRecoveryEfficiency() const {
    return m_heatRecoveryEfficiency;
  }

  void setHeatRecoveryEfficiency(double value) {
    m_heatRecoveryEfficiency = value;
  }

  /**
  * Fraction of supply air recirculated (unitless).
  */
  double exhaustAirRecirculated() const {
    return m_exhaustAirRecirculated;
  }

  void setExhaustAirRecirculated(double value) {
    m_exhaustAirRecirculated = value;
  }

  /**
  * Ventilation type (mechanical = 1.0, natural = 2.0, combined = 3.0).
  * XXX TODO: change this to an enum.
  */
  double type() const {
    return m_type;
  }

  void setType(double value) {
    m_type = value;
  }

  /**
  * Specific fan power (W/(L/s)).
  */
  double fanPower() const {
    return m_fanPower;
  }

  void setFanPower(double value) {
    m_fanPower = value;
  }

  /**
  * Fan flow control factor (unitless). This is the energy reduction from fan control measures.
  * 1 = no control, 0.75 = inlet blade adjuct, 0.65 = variable speed see NEN 2916 7.3.3.4.
  */
  double fanControlFactor() const {
    return m_fanControlFactor;
  }

  void setFanControlFactor(double value) {
    m_fanControlFactor = value;
  }

private:
  double m_supplyRate;
  double m_supplyDifference;
  double m_heatRecoveryEfficiency;
  double m_exhaustAirRecirculated;
  double m_type;
  double m_fanPower;
  double m_fanControlFactor;
};
} // isomodel
} // openstudio
#endif // ISOMODEL_VENTILATION_HPP
