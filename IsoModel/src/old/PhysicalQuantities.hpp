/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/
#ifndef ISOMODEL_PHYSICALQUANTITIES_HPP 
#define ISOMODEL_PHYSICALQUANTITIES_HPP

#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API PhysicalQuantities
{
public:
  // Use compiler-generated default constructor/destructor
  PhysicalQuantities() = default;
  ~PhysicalQuantities() = default;

  /**
  * Specific heat of air and water  in terms of volume (MJ/m3/K). Different parts of the simulation
  * use different units of RHO_CP_AIR. Multiply by 277.777778 to convert to watt-hr/m3/K.
  * Multiply by 1000000.0 to covert to W/m3/K.
  */
  double RHO_CP_AIR() const {
    return m_RHO_CP_AIR;
  }

  void setRHO_CP_AIR(double RHO_CP_AIR) {
    m_RHO_CP_AIR = RHO_CP_AIR;
  }

  double RHO_CP_WATER() const {
    return m_RHO_CP_WATER;
  }

  void setRHO_CP_WATER(double RHO_CP_WATER) {
    m_RHO_CP_WATER = RHO_CP_WATER;
  }

private:
  // In-class initialization (values preserved from original)
  double m_RHO_CP_AIR = 1.22521 * 0.001012;  // rho = 1.22521 kg/m3  * cp = 1.012 kJ / kg*K / 1000 kJ/MJ =   rhocp in MJ/(m^3 K)
  double m_RHO_CP_WATER = 4.1813;  // 4.1813 kJ/(m^3 k) / 1000 kJ/MJ
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_PHYSICALQUANTITIES_HPP