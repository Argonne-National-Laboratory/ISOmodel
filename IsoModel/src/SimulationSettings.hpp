/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/
#ifndef ISOMODEL_SIMULATIONSETTINGS_HPP 
#define ISOMODEL_SIMULATIONSETTINGS_HPP

#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API SimulationSettings
{
public:
  // Modernized: Use compiler-generated default constructor/destructor
  // This allows you to remove SimulationSettings.cpp safely.
  SimulationSettings() = default;
  ~SimulationSettings() = default;

  /**
  * Fraction of heat flow rate from interior sources that goes to the air node. ISO 13790 C.2 eq C.1
  * shows this as a constant 0.5.
  */
  double phiIntFractionToAirNode() const { return m_phiIntFractionToAirNode; }
  void setPhiIntFractionToAirNode(double val) { m_phiIntFractionToAirNode = val; }

  /**
  * Fraction of heat flow rate from solar that goes to the air node. ISO 13790 C.2 eq C.1
  * has no solar heat going directly to the air node (set to 0.0).
  */
  double phiSolFractionToAirNode() const { return m_phiSolFractionToAirNode; }
  void setPhiSolFractionToAirNode(double val) { m_phiSolFractionToAirNode = val; }

  /**
  * Default of 2.5 is used to generate the default values of h_is and h_ms found in ISO 13790.
  */
  double hci() const { return m_hci; }
  void setHci(double val) { m_hci = val; }

  /**
  * Default of 5.5 is used to generate the default values of h_is and h_ms found in ISO 13790.
  */
  double hri() const { return m_hri; }
  void setHri(double val) { m_hri = val; }

private:
  double m_phiIntFractionToAirNode = 0.5; // Default per ISO 13790 C.2 eq C.1.
  double m_phiSolFractionToAirNode = 0.0; // Default per ISO 13790 C.2 eq C.1.
  double m_hci = 2.5; 
  double m_hri = 5.5; 
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_SIMULATIONSETTINGS_HPP