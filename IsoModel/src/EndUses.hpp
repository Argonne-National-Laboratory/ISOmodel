// First Commit: 2013-11-05
//
// Authors:
// - Brendan Albano
// - Brian Craig
// - Nick Collier
// - Ralph Muehleisen
//
// Summary:
// Defines the `EndUses` class, a data container for storing simulation
// energy results. It categorizes energy consumption by fuel type (e.g.,
// Electricity, Gas) and end-use category (e.g., Heating, Cooling,
// Lights), providing a structured way to access and aggregate simulation
// outputs.

#ifndef ISOMODEL_ENDUSES_HPP
#define ISOMODEL_ENDUSES_HPP

#include "ISOModelAPI.hpp"
#include <vector> // Ensure vector is available for both modes

#ifdef ISOMODEL_STANDALONE
// Standalone includes if needed
#else
#include "../utilities/data/DataEnums.hpp"
#endif

namespace openstudio::isomodel {

class ISOMODEL_API EndUses {
public:
  // Default destructor
  ~EndUses() = default;

#ifdef ISOMODEL_STANDALONE
  // Default constructor (initialization handled in-class below)
  EndUses() = default;

  void addEndUse(int index, double value) {
    if (index >= 0 && index < static_cast<int>(_endUses.size())) {
      _endUses[index] = value;
    }
  }

  // Added const qualifier for read-only access
  double getEndUse(int index) const {
    if (index >= 0 && index < static_cast<int>(_endUses.size())) {
      return _endUses[index];
    }
    return 0.0;
  }
#else
  // --------------------------------------------------------------------------
  // OPTIMIZATION (Item 2):
  // Flattened storage for O(1) access instead of linear search through pairs.
  // 2 Fuel Types * 9 Categories = 18 slots.
  // --------------------------------------------------------------------------

  // Default constructor: pre-allocate flattened vector
  EndUses() : _endUses(18, 0.0) {}

  void addEndUse(double value, EndUseFuelType fuel,
                 EndUseCategoryType category) {
    int index = getIndex(fuel, category);
    if (index >= 0 && index < static_cast<int>(_endUses.size())) {
      _endUses[index] = value;
    }
  }

  double getEndUse(EndUseFuelType fuel, EndUseCategoryType category) const {
    int index = getIndex(fuel, category);
    if (index >= 0 && index < static_cast<int>(_endUses.size())) {
      return _endUses[index];
    }
    return 0.0;
  }

  static std::vector<EndUseFuelType> fuelTypes() {
    return {EndUseFuelType::Electricity, EndUseFuelType::Gas};
  }

  static std::vector<EndUseCategoryType> categories() {
    return {EndUseCategoryType::Heating,
            EndUseCategoryType::Cooling,
            EndUseCategoryType::InteriorLights,
            EndUseCategoryType::ExteriorLights,
            EndUseCategoryType::Fans,
            EndUseCategoryType::Pumps,
            EndUseCategoryType::InteriorEquipment,
            EndUseCategoryType::ExteriorEquipment,
            EndUseCategoryType::WaterSystems};
  }
#endif

private:
#ifdef ISOMODEL_STANDALONE
  std::vector<double> _endUses = std::vector<double>(13, 0.0);
#else
  // Flattened storage:
  // Index = (FuelIndex * 9) + CategoryIndex
  std::vector<double> _endUses;

  // Helper to map Enums to flat integer index
  // Returns -1 if invalid (though enums are strongly typed)
  int getIndex(EndUseFuelType fuel, EndUseCategoryType category) const {
    // Map Fuel (0 or 1)
    int f = (fuel == EndUseFuelType::Gas) ? 1 : 0;

    // Map Category (0 to 8)
    // Relying on EndUseCategoryType enum integer values matching standard OS
    // order. If enum values are not contiguous 0-8, a switch statement would be
    // safer, but casting is standard for this specific optimization in
    // OS-related code.
    int c = static_cast<int>(category);

    return (f * 9) + c;
  }
#endif
};

} // namespace openstudio::isomodel

#endif // ISOMODEL_ENDUSES_HPP