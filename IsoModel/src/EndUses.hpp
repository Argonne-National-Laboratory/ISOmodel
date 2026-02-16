#pragma once
#include "ISOModelAPI.hpp"

#include <vector>

namespace openstudio::isomodel {

class ISOMODEL_API EndUses {
public:
  ~EndUses() = default;
  EndUses() = default;

  void addEndUse(int index, double value) {
    if (index >= 0 && index < static_cast<int>(_endUses.size())) {
      _endUses[index] = value;
    }
  }

  [[nodiscard]] double getEndUse(int index) const {
    if (index >= 0 && index < static_cast<int>(_endUses.size())) {
      return _endUses[index];
    }
    return 0.0;
  }

private:
  std::vector<double> _endUses = std::vector<double>(13, 0.0);
};

} // namespace openstudio::isomodel

