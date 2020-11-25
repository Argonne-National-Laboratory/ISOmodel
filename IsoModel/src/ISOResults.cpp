/**********************************************************************
 *  Copyright (c) 2008-2015, Alliance for Sustainable Energy.
 *  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/

#include "ISOResults.hpp"

namespace openstudio {
namespace isomodel {

double totalEnergyUse(const std::vector<EndUses>& results) {
  auto total = 0.0;

#ifdef ISOMODEL_STANDALONE
  for (const auto& result : results) {
    for (int i = 0; i < 12; i++) {
       total += result.getEndUse(i);
    }
  }
#else
  const auto fuelTypes = EndUses::fuelTypes();
  const auto categories = EndUses::categories();

  for (const auto& result : results) {
    for (const auto& fuelType : fuelTypes) {
      for (const auto& category : categories) {
        total += result.getEndUse(fuelType, category);
      }
    }
  }
#endif

  return total;
}

}
}

