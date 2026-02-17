/// @file ISOResults.hpp
/// @brief Simulation result container for monthly energy totals.
///
/// Holds the vector of EndUses results returned by MonthlyModel::simulate()
/// and HourlyModel::simulate().
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#pragma once
#include "ISOModelAPI.hpp"

#include <vector>

#include "EndUses.hpp"

namespace openstudio::isomodel {

/// Sums the energy use from the results of an ISOModel simulation across all
/// timesteps and EndUses.
[[nodiscard]] ISOMODEL_API double totalEnergyUse(const std::vector<EndUses> &results);

} // namespace openstudio::isomodel

