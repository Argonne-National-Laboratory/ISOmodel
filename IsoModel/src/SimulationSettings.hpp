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
#ifndef ISOMODEL_SIMULATIONSETTINGS_HPP 
#define ISOMODEL_SIMULATIONSETTINGS_HPP

#include "ISOModelAPI.hpp"

namespace openstudio {
namespace isomodel {
class ISOMODEL_API SimulationSettings
{
public:
  SimulationSettings(void);
  ~SimulationSettings(void);

  /**
  * Fraction of heat flow rate from interior sources that goes to the air node. ISO 13790 C.2 eq C.1
  * shows this as a constant 0.5.
  * \Phi_{st} and \Phi_{m} are calculated differently than in ISO 13790 to
  * allow variation in the values that factor the amount of interior and solar
  * heat gain that heats the air. These variables are used in those
  * calculations.
  */
  double phiIntFractionToAirNode() const {
    return m_phiIntFractionToAirNode;
  }

  void setPhiIntFractionToAirNode(double phiIntFractionToAirNode) {
    m_phiIntFractionToAirNode = phiIntFractionToAirNode;
  }

  /**
  * Fraction of heat flow rate from solar that goes to the air node. ISO 13790 C.2 eq C.1
  * has no solar heat going directly to the air node (set to 0.0).
  * \Phi_{st} and \Phi_{m} are calculated differently than in ISO 13790 to
  * allow variation in the values that factor the amount of interior and solar
  * heat gain that heats the air. These variables are used in those
  * calculations.
  */
  double phiSolFractionToAirNode() const {
    return m_phiSolFractionToAirNode;
  }

  void setPhiSolFractionToAirNode(double phiSolFractionToAirNode) {
    m_phiSolFractionToAirNode = phiSolFractionToAirNode;
  }

  /**
  * Default of 2.5 is used to generate the default values of h_is and h_ms found in ISO 13790.
  */
  double hci() const {
    return m_hci;
  }

  void setHci(double hci) {
    m_hci = hci;
  }

  /**
  * Default of 5.5 is used to generate the default values of h_is and h_ms found in ISO 13790.
  */
  double hri() const {
    return m_hri;
  }

  void setHri(double hri) {
    m_hri = hri;
  }

private:
  double m_phiIntFractionToAirNode = 0.5; // Default value is the "0.5" from ISO 13790 C.2 eq C.1.
  double m_phiSolFractionToAirNode = 0; // Default is that no solar heat flows directly to air node per ISO 13790 C.2 eq C.1.
  double m_hci = 2.5; // Default of 2.5 is used to generate the default values of h_is and h_ms found in ISO 13790.
  double m_hri = 5.5; // Default of 5.5 is used to generate the default values of h_is and h_ms found in ISO 13790.
};

} // isomodel
} // openstudio
#endif // ISOMODEL_SIMULATIONSETTINGS_HPP
