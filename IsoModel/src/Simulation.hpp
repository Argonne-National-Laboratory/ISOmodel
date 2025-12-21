#ifndef ISOMODEL_SIMULATION_HPP
#define ISOMODEL_SIMULATION_HPP

#include "ISOModelAPI.hpp"

#include "Population.hpp"
#include "Location.hpp"
#include "Lighting.hpp"
#include "Building.hpp"
#include "Cooling.hpp"
#include "Heating.hpp"
#include "Structure.hpp"
#include "Ventilation.hpp"
#include "EpwData.hpp"
#include "PhysicalQuantities.hpp"
#include "SimulationSettings.hpp"

#include <memory>

namespace openstudio::isomodel {

class ISOMODEL_API Simulation
{
public:
  Simulation() = default;
  virtual ~Simulation() = default;

  // Setters for the pointers to the classes that store the .ism parameters.
  void setPop(const Population& value) { pop = value; }
  void setLocation(const Location& value) { location = value; }
  void setLights(const Lighting& value) { lights = value; }
  void setBuilding(const Building& value) { building = value; }
  void setStructure(const Structure& value) { structure = value; }
  void setHeating(const Heating& value) { heating = value; }
  void setCooling(const Cooling& value) { cooling = value; }
  void setVentilation(const Ventilation& value) { ventilation = value; }
  
  void setEpwData(std::shared_ptr<EpwData> value) { epwData = value; }
  void setPhysicalQuantities(const PhysicalQuantities& value) { phys = value; }
  void setSimulationSettings(const SimulationSettings& value) { simSettings = value; }

protected:
  // Pointers/Objects that store the .ism parameters.
  Population pop;
  Location location;
  Lighting lights;
  Building building;
  Structure structure;
  Heating heating;
  Cooling cooling;
  Ventilation ventilation;
  std::shared_ptr<EpwData> epwData;
  PhysicalQuantities phys;
  SimulationSettings simSettings;
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_SIMULATION_HPP