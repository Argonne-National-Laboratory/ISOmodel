/**********************************************************************
 * Copyright (c) 2008-2015, Alliance for Sustainable Energy.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/

#ifndef ISOMODEL_ENDUSES_HPP
#define ISOMODEL_ENDUSES_HPP

#include "ISOModelAPI.hpp"

#ifdef ISOMODEL_STANDALONE
#include <vector>
#else
#include "../utilities/data/DataEnums.hpp"
#endif

namespace openstudio::isomodel {

class ISOMODEL_API EndUses
{
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
  // Default constructor
  EndUses() = default;

  void addEndUse(double value, EndUseFuelType fuel, EndUseCategoryType category) {
    _endUses.push_back(std::make_pair(std::make_pair(fuel, category), value));
  }

  // Added const qualifier for read-only access
  double getEndUse(EndUseFuelType fuel, EndUseCategoryType category) const {
    for (auto const& endUse : _endUses) {
      if (endUse.first.first == fuel && endUse.first.second == category) {
        return endUse.second;
      }
    }
    return 0.0;
  }

  static std::vector<EndUseFuelType> fuelTypes() {
    return { EndUseFuelType::Electricity, EndUseFuelType::Gas };
  }

  static std::vector<EndUseCategoryType> categories() {
    return { EndUseCategoryType::Heating, EndUseCategoryType::Cooling, EndUseCategoryType::InteriorLights,
             EndUseCategoryType::ExteriorLights, EndUseCategoryType::Fans, EndUseCategoryType::Pumps,
             EndUseCategoryType::InteriorEquipment, EndUseCategoryType::ExteriorEquipment, EndUseCategoryType::WaterSystems };
  }
#endif

private:
#ifdef ISOMODEL_STANDALONE
  // In-class initialization replaces constructor initialization list
  std::vector<double> _endUses = std::vector<double>(13, 0.0);
#else
  std::vector<std::pair<std::pair<EndUseFuelType, EndUseCategoryType>, double>> _endUses;
#endif

};

} // namespace openstudio::isomodel

#endif // ISOMODEL_ENDUSES_HPP