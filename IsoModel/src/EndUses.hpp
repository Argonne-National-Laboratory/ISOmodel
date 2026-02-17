/// @file EndUses.hpp
/// @brief Energy end-use category indices for simulation results.
///
/// Defines the EndUses enum with indices for heating, cooling, interior
/// lighting, exterior lighting, interior equipment, exterior equipment,
/// fans, pumps, hot water, and other fuel types. Used to index into
/// the result vectors returned by MonthlyModel and HourlyModel.
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

namespace openstudio::isomodel {

class ISOMODEL_API EndUses {
public:
  ~EndUses() = default;
  EndUses() = default;

  void addEndUse(int index, double value) {
    if (index >= 0 && index < static_cast<int>(m_endUses.size())) {
      m_endUses[index] = value;
    }
  }

  [[nodiscard]] double getEndUse(int index) const noexcept {
    if (index >= 0 && index < static_cast<int>(m_endUses.size())) {
      return m_endUses[index];
    }
    return 0.0;
  }

private:
  std::vector<double> m_endUses = std::vector<double>(13, 0.0);
};

} // namespace openstudio::isomodel
