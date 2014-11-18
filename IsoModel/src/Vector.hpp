#ifndef ISOMODEL_VECTOR_HPP
#define ISOMODEL_VECTOR_HPP
#include <boost/numeric/ublas/vector.hpp>

namespace openstudio {
/// Workaround to get Vector typedef, http://www.gotw.ca/gotw/079.htm
struct VectorBypass
{
  typedef boost::numeric::ublas::vector<double> VectorType;
};
/// Vector 
typedef VectorBypass::VectorType Vector;
}
#endif
