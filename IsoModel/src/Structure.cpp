#include "Structure.hpp"

namespace openstudio::isomodel {

// Explicitly default constructor and destructor in the source file
// to ensure linker symbols are generated for the exported class.
Structure::Structure() = default;
Structure::~Structure() = default;

} // namespace openstudio::isomodel
