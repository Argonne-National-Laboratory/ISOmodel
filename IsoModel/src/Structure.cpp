#include "Structure.hpp"

namespace openstudio {
namespace isomodel {

Structure::Structure() : m_wallArea(9, 0), m_windowArea (9, 0), m_wallUniform(9, 0), m_windowUniform(9, 0),
    m_wallThermalEmissivity(9, 0), m_wallSolarAbsorbtion(9, 0), m_windowShadingDevice(9, 0),
    m_windowNormalIncidenceSolarEnergyTransmittance(9, 0), m_windowShadingCorrectionFactor(9, 0)
{
}

Structure::~Structure()
{
}

} // isomodel
} // openstudio
