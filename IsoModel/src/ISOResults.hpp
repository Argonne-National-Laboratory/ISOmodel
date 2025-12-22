/**********************************************************************
 * Copyright (c) 2008-2015, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/

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

/// Sums the energy use from the results of an ISOModel simulation across all timesteps and EndUses.
ISOMODEL_API double totalEnergyUse(const std::vector<EndUses>& results);

} // namespace openstudio::isomodel

#endif // ISOMODEL_ISORESULTS_HPP