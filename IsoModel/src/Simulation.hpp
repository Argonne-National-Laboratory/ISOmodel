#ifndef ISOMODEL_SIMULATION_HPP
#define ISOMODEL_SIMULATION_HPP

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

namespace openstudio {
namespace isomodel {

class Simulation
{
public:
  virtual ~Simulation() {}

  // Setters for the pointers to the classes that store the .ism parameters.
  void setPop(Population value) {
    pop = value;
  }

  void setLocation(Location value) {
    location = value;
  }

  void setLights(Lighting value) {
    lights = value;
  }

  void setBuilding(Building value) {
    building = value;
  }

  void setStructure(Structure value) {
    structure = value;
  }

  void setHeating(Heating value) {
    heating = value;
  }

  void setCooling(Cooling value) {
    cooling = value;
  }

  void setVentilation(Ventilation value) {
    ventilation = value;
  }
  
  void setEpwData(std::shared_ptr<EpwData> value) {
    epwData = value;
  }

  void setPhysicalQuantities(PhysicalQuantities value) {
    phys = value;
  }

  void setSimulationSettings(SimulationSettings value) {
    simSettings = value;
  }

protected:
  // Pointers to classes that store the .ism parameters.
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
} // isomodel
} // openstudio
#endif ISOMODEL_SIMULATION_HPP