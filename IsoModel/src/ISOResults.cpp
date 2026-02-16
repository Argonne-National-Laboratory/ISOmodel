
#include "ISOResults.hpp"

#include <vector>

namespace openstudio::isomodel {

double totalEnergyUse(const std::vector<EndUses> &results) {
  double total = 0.0;

  for (const auto &result : results) {
    for (int i = 0; i < 13; ++i) {
      total += result.getEndUse(i);
    }
  }

  return total;
}

} // namespace openstudio::isomodel