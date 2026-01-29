// First Commit: 2013-11-05
//
// Authors:
// - Brendan Albano
// - Brian Craig
// - Nick Collier
// - Ralph Muehleisen
//
// Summary:
// This file implements the `totalEnergyUse` helper function, which
// aggregates the total energy consumption from a collection of `EndUses`
// results objects. It supports both standalone and OpenStudio-integrated
// `EndUses` structures.

#include "ISOResults.hpp"
#include <vector>

namespace openstudio::isomodel {

double totalEnergyUse(const std::vector<EndUses> &results) {
  double total = 0.0;

  for (const auto &result : results) {
#ifdef ISOMODEL_STANDALONE
    for (int i = 0; i < 13; ++i) {
      total += result.getEndUse(i);
    }
#else
    const auto fuelTypes = EndUses::fuelTypes();
    const auto categories = EndUses::categories();

    for (const auto &fuelType : fuelTypes) {
      for (const auto &category : categories) {
        total += result.getEndUse(fuelType, category);
      }
    }
#endif
  }

  return total;
}

} // namespace openstudio::isomodel