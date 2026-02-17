/// @file Simulation.hpp
/// @brief Abstract base class for HourlyModel and MonthlyModel.
///
/// Holds shared references to the component property objects (Population,
/// Location, Building, Structure, etc.) that both simulation engines need.
/// Provides the common interface for setting up and running simulations.
///
/// @author Brendan Albano
/// @author Nick Collier
/// @author Ralph Muehleisen
/// @date 2015-06-01
/// @copyright Copyright Argonne National Laboratory
#pragma once
#include "Building.hpp"
#include "Cooling.hpp"
#include "EpwData.hpp"
#include "Heating.hpp"
#include "ISOModelAPI.hpp"
#include "Lighting.hpp"
#include "Location.hpp"
#include "Population.hpp"
#include "Structure.hpp"
#include "Ventilation.hpp"
#include "SimulationSettings.hpp"

#include <memory>

namespace openstudio::isomodel {

class ISOMODEL_API Simulation {
public:
  Simulation() = default;
  virtual ~Simulation() = default;

  // Setters for the pointers to the classes that store the .ism parameters.
  void setPop(const Population &value) { m_pop = value; }
  void setLocation(const Location &value) { m_location = value; }
  void setLights(const Lighting &value) { m_lights = value; }
  void setBuilding(const Building &value) { m_building = value; }
  void setStructure(const Structure &value) { m_structure = value; }
  void setHeating(const Heating &value) { m_heating = value; }
  void setCooling(const Cooling &value) { m_cooling = value; }
  void setVentilation(const Ventilation &value) { m_ventilation = value; }

  void setEpwData(std::shared_ptr<EpwData> value) { m_epwData = std::move(value); }
  void setSimulationSettings(const SimulationSettings &value) { m_simSettings = value; }

protected:
  Population m_pop;
  Location m_location;
  Lighting m_lights;
  Building m_building;
  Structure m_structure;
  Heating m_heating;
  Cooling m_cooling;
  Ventilation m_ventilation;
  std::shared_ptr<EpwData> m_epwData;
  SimulationSettings m_simSettings;
};

} // namespace openstudio::isomodel
