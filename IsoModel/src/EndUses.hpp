#ifndef ISOMODEL_ENDUSES_HPP
#define ISOMODEL_ENDUSES_HPP

#include <vector>
#include "ISOModelAPI.hpp"

namespace openstudio {
class ISOMODEL_API EndUses
{
protected:
  std::vector<double> data;
public:
  EndUses() {data.resize(20);}
  ~EndUses(void) {}
  inline void addEndUse(int use, double value) {
    data[use] = value;
  }
  inline double getEndUse(int use) const {
    return data[use];
  }
};
}
#endif
