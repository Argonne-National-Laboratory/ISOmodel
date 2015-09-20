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

// TODO: make a version of this for the standalone isomodel. BAA@2015-08-17.
double totalEnergyUse(const std::vector<EndUses>& results) {
  auto total = 0.0;
  const auto fuelTypes = EndUses::fuelTypes();
  const auto categories = EndUses::categories();

  for (const auto& result : results) {
    for (const auto& fuelType : fuelTypes) {
      for (const auto& category : categories) {
        total += result.getEndUse(fuelType, category);
      }
    }
  }

  return total;
}

}
}

