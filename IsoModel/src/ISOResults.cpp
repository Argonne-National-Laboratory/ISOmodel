/**********************************************************************
 * Copyright (c) 2008-2015, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/

#include "ISOResults.hpp"
#include <vector>

namespace openstudio::isomodel {

double totalEnergyUse(const std::vector<EndUses>& results) {
  double total = 0.0;

  for (const auto& result : results) {
#ifdef ISOMODEL_STANDALONE
    for (int i = 0; i < 13; ++i) {
      total += result.getEndUse(i);
    }
#else
    const auto fuelTypes = EndUses::fuelTypes();
    const auto categories = EndUses::categories();

    for (const auto& fuelType : fuelTypes) {
      for (const auto& category : categories) {
        total += result.getEndUse(fuelType, category);
      }
    }
#endif
  }

  return total;
}

} // namespace openstudio::isomodel