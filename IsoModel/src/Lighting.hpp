#ifndef ISOMODEL_LIGHTING_HPP
#define ISOMODEL_LIGHTING_HPP

#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

class ISOMODEL_API Lighting
{
public:
  Lighting() = default;
  ~Lighting() = default;

  double powerDensityOccupied() const { return m_powerDensityOccupied; }
  void setPowerDensityOccupied(double value) { m_powerDensityOccupied = value; }

  double powerDensityUnoccupied() const { return m_powerDensityUnoccupied; }
  void setPowerDensityUnoccupied(double value) { m_powerDensityUnoccupied = value; }

  double dimmingFraction() const { return m_dimmingFraction; }
  void setDimmingFraction(double value) { m_dimmingFraction = value; }

  double exteriorEnergy() const { return m_exteriorEnergy; }
  void setExteriorEnergy(double value) { m_exteriorEnergy = value; }

  double n_day_start() const { return m_n_day_start; }
  void setN_day_start(double value) { m_n_day_start = value; }

  double n_day_end() const { return m_n_day_end; }
  void setN_day_end(double value) { m_n_day_end = value; }

  double n_weeks() const { return m_n_weeks; }
  void setN_weeks(double value) { m_n_weeks = value; }

  double elecInternalGains() const { return m_elecInternalGains; }
  void setElecInternalGains(double value) { m_elecInternalGains = value; }

  double permLightPowerDensity() const { return m_permLightPowerDensity; }
  void setPermLightPowerDensity(double value) { m_permLightPowerDensity = value; }

  double presenceSensorAd() const { return m_presenceSensorAd; }
  void setPresenceSensorAd(double value) { m_presenceSensorAd = value; }

  double automaticAd() const { return m_automaticAd; }
  void setAutomaticAd(double value) { m_automaticAd = value; }

  double presenceAutoAd() const { return m_presenceAutoAd; }
  void setPresenceAutoAd(double value) { m_presenceAutoAd = value; }

  double manualSwitchAd() const { return m_manualSwitchAd; }
  void setManualSwitchAd(double value) { m_manualSwitchAd = value; }

  double presenceSensorLux() const { return m_presenceSensorLux; }
  void setPresenceSensorLux(double value) { m_presenceSensorLux = value; }

  double automaticLux() const { return m_automaticLux; }
  void setAutomaticLux(double value) { m_automaticLux = value; }

  double presenceAutoLux() const { return m_presenceAutoLux; }
  void setPresenceAutoLux(double value) { m_presenceAutoLux = value; }

  double manualSwitchLux() const { return m_manualSwitchLux; }
  void setManualSwitchLux(double value) { m_manualSwitchLux = value; }

  double naturallyLightedArea() const { return m_naturallyLightedArea; }
  void setNaturallyLightedArea(double value) { m_naturallyLightedArea = value; }

  double lightingPowerFixedOccupied() const { return m_lightingPowerFixedOccupied; }
  void setLightingPowerFixedOccupied(double value) { m_lightingPowerFixedOccupied = value; }

  double lightingPowerFixedUnoccupied() const { return m_lightingPowerFixedUnoccupied; }
  void setLightingPowerFixedUnoccupied(double value) { m_lightingPowerFixedUnoccupied = value; }

private:
  double m_powerDensityOccupied = 0.0;
  double m_powerDensityUnoccupied = 0.0;
  double m_dimmingFraction = 0.0;
  double m_exteriorEnergy = 0.0;
  
  // Default values
  double m_n_day_start = 7.0; 
  double m_n_day_end = 18.0; 
  double m_n_weeks = 50.0;
  double m_elecInternalGains = 1.0;
  double m_permLightPowerDensity = 0.0;
  
  // Automatic lighting control defaults:
  double m_presenceSensorAd = 0.6;
  double m_automaticAd = 0.8;
  double m_presenceAutoAd = 0.6;
  double m_manualSwitchAd = 1.0;
  double m_presenceSensorLux = 500.0;
  double m_automaticLux = 300.0;
  double m_presenceAutoLux = 300.0;
  double m_manualSwitchLux = 500.0;
  // Daylighting
  double m_naturallyLightedArea = 0.0;

  double m_lightingPowerFixedOccupied = 0.0;
  double m_lightingPowerFixedUnoccupied = 0.0;
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_LIGHTING_HPP