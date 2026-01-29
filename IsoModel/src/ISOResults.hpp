// First Commit: 2013-11-05
//
// Authors:
// - Brendan Albano
// - Brian Craig
// - Nick Collier
// - Ralph Muehleisen
//
// Summary:
// This file defines helper functions for processing simulation results. It
// includes utilities like `totalEnergyUse` to aggregate energy consumption
// data from a vector of `EndUses` objects.

#ifndef ISOMODEL_ISORESULTS_HPP
#define ISOMODEL_ISORESULTS_HPP

#include "ISOModelAPI.hpp"
#include <vector>

#ifdef ISOMODEL_STANDALONE
#include "EndUses.hpp"
#else
#include "../utilities/data/EndUses.hpp"
#endif

namespace openstudio::isomodel {

/// Sums the energy use from the results of an ISOModel simulation across all
/// timesteps and EndUses.
ISOMODEL_API double totalEnergyUse(const std::vector<EndUses> &results);

} // namespace openstudio::isomodel

#endif // ISOMODEL_ISORESULTS_HPP