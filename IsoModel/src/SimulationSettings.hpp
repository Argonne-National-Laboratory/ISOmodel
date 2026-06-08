/// @file SimulationSettings.hpp
/// @brief ISO 13790 simulation parameters for the 5R1C thermal network.
///
/// Stores the internal/solar heat flow distribution fractions (phi_int_is,
/// phi_sol_is) and the surface-to-air heat transfer ratios (h_is, h_ms)
/// used in the hourly 5R1C model. Default values follow ISO 13790 §7.2.2.
///
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2015-06-12
/// @copyright Copyright Argonne National Laboratory
#pragma once
#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API SimulationSettings {
public:
  // Modernized: Use compiler-generated default constructor/destructor
  // This allows you to remove SimulationSettings.cpp safely.
  SimulationSettings() = default;
  ~SimulationSettings() = default;

  /// Fraction of heat flow rate from interior sources that goes to the air node.
  /// ISO 13790 C.2 eq C.1 shows this as a constant 0.5.
  [[nodiscard]] double phiIntFractionToAirNode() const noexcept {
    return m_phiIntFractionToAirNode;
  }
  void setPhiIntFractionToAirNode(double val) { m_phiIntFractionToAirNode = val; }

  /// Fraction of heat flow rate from solar that goes to the air node. ISO 13790
  /// C.2 eq C.1 has no solar heat going directly to the air node (set to 0.0).
  [[nodiscard]] double phiSolFractionToAirNode() const noexcept {
    return m_phiSolFractionToAirNode;
  }
  void setPhiSolFractionToAirNode(double val) { m_phiSolFractionToAirNode = val; }

  /// Default of 2.5 is used to generate the default values of h_is and h_ms
  /// found in ISO 13790.
  [[nodiscard]] double hci() const noexcept { return m_hci; }
  void setHci(double val) { m_hci = val; }

  /// Default of 5.5 is used to generate the default values of h_is and h_ms
  /// found in ISO 13790.
  [[nodiscard]] double hri() const noexcept { return m_hri; }
  void setHri(double val) { m_hri = val; }

private:
  double m_phiIntFractionToAirNode = 0.5; // Default per ISO 13790 C.2 eq C.1.
  double m_phiSolFractionToAirNode = 0.0; // Default per ISO 13790 C.2 eq C.1.
  double m_hci = 2.5;
  double m_hri = 5.5;
};

} // namespace openstudio::isomodel
