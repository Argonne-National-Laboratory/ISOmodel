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

  // Getters.
  double supplyRate() const {
    return m_supplyRate;
  }

  double supplyDifference() const {
    return m_supplyDifference;
  }

  double heatRecoveryEfficiency() const {
    return m_heatRecoveryEfficiency;
  }

  double exhaustAirRecirculated() const {
    return m_exhaustAirRecirculated;
  }

  double type() const {
    return m_type;
  }

  double fanPower() const {
    return m_fanPower;
  }

  double fanControlFactor() const {
    return m_fanControlFactor;
  }

  double wasteFactor() const {
    return m_wasteFactor;
  }

  // Setters.
  void setSupplyRate(double value) {
    m_supplyRate = value;
  }

  void setSupplyDifference(double value) {
    m_supplyDifference = value;
  }

  void setHeatRecoveryEfficiency(double value) {
    m_heatRecoveryEfficiency = value;
  }

  void setExhaustAirRecirculated(double value) {
    m_exhaustAirRecirculated = value;
  }

  void setType(double value) {
    m_type = value;
  }

  void setFanPower(double value) {
    m_fanPower = value;
  }

  void setFanControlFactor(double value) {
    m_fanControlFactor = value;
  }

  void setWasteFactor(double value) {
    m_wasteFactor = value;
  }

private:
  double m_supplyRate;
  double m_supplyDifference;
  double m_heatRecoveryEfficiency;
  double m_exhaustAirRecirculated;
  double m_type;
  double m_fanPower;
  double m_fanControlFactor;
  double m_wasteFactor;
};
} // isomodel
} // openstudio
#endif // ISOMODEL_VENTILATION_HPP
