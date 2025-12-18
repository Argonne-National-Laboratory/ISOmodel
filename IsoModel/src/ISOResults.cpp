/**********************************************************************
 * Copyright (c) 2008-2015, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/

#include "ISOResults.hpp"
#include <vector>

namespace openstudio {
    namespace isomodel {

        double totalEnergyUse(const std::vector<EndUses>& results) {
            auto total = 0.0;

            // Create a non-const copy or use a non-const loop if the getter isn't thread-safe/const
            // Since results is const&, we cast away constness locally to call the getter 
            // if we cannot modify EndUses.hpp.
            for (const auto& result : results) {
                auto& mutableResult = const_cast<EndUses&>(result);
#ifdef ISOMODEL_STANDALONE
                for (int i = 0; i < 13; ++i) {
                    total += mutableResult.getEndUse(i);
                }
#else
                const auto fuelTypes = EndUses::fuelTypes();
                const auto categories = EndUses::categories();

                for (const auto& fuelType : fuelTypes) {
                    for (const auto& category : categories) {
                        total += mutableResult.getEndUse(fuelType, category);
                    }
                }
#endif
            }

            return total;
        }

    } // namespace isomodel
} // namespace openstudio