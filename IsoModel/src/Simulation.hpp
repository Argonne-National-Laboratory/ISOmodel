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

namespace openstudio {
namespace isomodel {

class Simulation
{
public:
  virtual ~Simulation() {}

  // Setters for the pointers to the classes that store the .ism parameters.
  void setPop(std::shared_ptr<Population> value) {
    pop = value;
  }

  void setLocation(std::shared_ptr<Location> value) {
    location = value;
  }

  void setLights(std::shared_ptr<Lighting> value) {
    lights = value;
  }

  void setBuilding(std::shared_ptr<Building> value) {
    building = value;
  }

  void setStructure(std::shared_ptr<Structure> value) {
    structure = value;
  }

  void setHeating(std::shared_ptr<Heating> value) {
    heating = value;
  }

  void setCooling(std::shared_ptr<Cooling> value) {
    cooling = value;
  }

  void setVentilation(std::shared_ptr<Ventilation> value) {
    ventilation = value;
  }
  
  void setEpwData(std::shared_ptr<EpwData> value) {
    epwData = value;
  }

protected:
  // Pointers to classes that store the .ism parameters.
  std::shared_ptr<Population> pop;
  std::shared_ptr<Location> location;
  std::shared_ptr<Lighting> lights;
  std::shared_ptr<Building> building;
  std::shared_ptr<Structure> structure;
  std::shared_ptr<Heating> heating;
  std::shared_ptr<Cooling> cooling;
  std::shared_ptr<Ventilation> ventilation;
  std::shared_ptr<EpwData> epwData;
};
} // isomodel
} // openstudio
#endif ISOMODEL_SIMULATION_HPP