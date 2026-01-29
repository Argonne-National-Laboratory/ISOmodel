// First Commit: 2013-11-05
//
// Authors:
// - Brendan Albano
// - Brian Craig
// - Nick Collier
// - Ralph Muehleisen
//
// Summary:
// Defines the legacy `PhysicalQuantities` class, which was used to hold
// physical constants like the volumetric heat capacity of air and water.
// This class has been deprecated in favor of centralized constants in
// `Constants.hpp`.

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