
#pragma once
#include "ISOModelAPI.hpp"

#include <vector>

#include "EndUses.hpp"

namespace openstudio::isomodel {

/// Sums the energy use from the results of an ISOModel simulation across all
/// timesteps and EndUses.
[[nodiscard]] ISOMODEL_API double totalEnergyUse(const std::vector<EndUses> &results);

} // namespace openstudio::isomodel

