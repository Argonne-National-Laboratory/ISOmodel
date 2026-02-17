/// @file Structure.cpp
/// @brief Building envelope properties: areas, U-values, SHGC, and thermal mass.
///
/// Stores wall/window/roof areas, U-values, solar heat gain coefficients,
/// thermal emissivity, solar absorption, shading device factors, interior
/// and exterior heat capacities, building height, and infiltration rate.
/// Properties are organized as 9-element arrays for the 8 cardinal/
/// intercardinal orientations plus the roof.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#include "Structure.hpp"

namespace openstudio::isomodel {

// Explicitly default constructor and destructor in the source file
// to ensure linker symbols are generated for the exported class.
Structure::Structure() = default;
Structure::~Structure() = default;

} // namespace openstudio::isomodel
