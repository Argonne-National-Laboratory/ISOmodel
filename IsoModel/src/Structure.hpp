/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/
#ifndef ISOMODEL_STRUCTURE_HPP
#define ISOMODEL_STRUCTURE_HPP

#include "ISOModelAPI.hpp"

#ifdef ISOMODEL_STANDALONE
//#include "Vector.hpp"
#include "MathHelpers.hpp"
#else
#include "../utilities/data/Vector.hpp"
#endif

namespace openstudio::isomodel {

class ISOMODEL_API Structure
{
public:
  // Fix: Declare here, define in cpp
  Structure();
  ~Structure();

  /**
  * Floor area (m2).
  */
  double floorArea() const { return m_floorArea; }
  void setFloorArea(double value) { m_floorArea = value; }

  /**
  * Wall and roof area (m2). The order is S, SE, E, NE, N, NW, W, SW, roof.
  */
  Vector wallArea() const { return m_wallArea; }
  void setWallArea(const Vector& value) { m_wallArea = value; }
  void setWallArea(int index, double value) { m_wallArea[index] = value; }

  /**
  * Window and skylight area (m2). The order is S, SE, E, NE, N, NW, W, SW, roof.
  */
  Vector windowArea() const { return m_windowArea; }
  void setWindowArea(const Vector& value) { m_windowArea = value; }
  void setWindowArea(int index, double value) { m_windowArea[index] = value; }

  /**
  * Wall and roof U-values (W/m2/K).
  */
  Vector wallUniform() const { return m_wallUniform; }
  void setWallUniform(const Vector& value) { m_wallUniform = value; }
  void setWallUniform(int index, double value) { m_wallUniform[index] = value; }

  /**
  * Window and skylight U-values (W/m2/K).
  */
  Vector windowUniform() const { return m_windowUniform; }
  void setWindowUniform(const Vector& value) { m_windowUniform = value; }
  void setWindowUniform(int index, double value) { m_windowUniform[index] = value; }

  /**
  * Wall and roof thermal emissivity (0 to 1).
  */
  Vector wallThermalEmissivity() const { return m_wallThermalEmissivity; }
  void setWallThermalEmissivity(const Vector& value) { m_wallThermalEmissivity = value; }
  void setWallThermalEmissivity(int index, double value) { m_wallThermalEmissivity[index] = value; }

  /**
  * Wall and roof solar absorption coeficient (0 to 1).
  */
  Vector wallSolarAbsorption() const { return m_wallSolarAbsorbtion; }
  void setWallSolarAbsorption(const Vector& value) { m_wallSolarAbsorbtion = value; }
  void setWallSolarAbsorption(int index, double value) { m_wallSolarAbsorbtion[index] = value; }

  /**
  * Window shading device factors.
  */
  Vector windowShadingDevice() const { return m_windowShadingDevice; }
  void setWindowShadingDevice(const Vector& value) { m_windowShadingDevice = value; }
  void setWindowShadingDevice(int index, double value) { m_windowShadingDevice[index] = value; }

  /**
  * Window solar heat gain coeficcient (0 to 1).
  */
  Vector windowNormalIncidenceSolarEnergyTransmittance() const { return m_windowNormalIncidenceSolarEnergyTransmittance; }
  void setWindowNormalIncidenceSolarEnergyTransmittance(const Vector& value) { m_windowNormalIncidenceSolarEnergyTransmittance = value; }
  void setWindowNormalIncidenceSolarEnergyTransmittance(int index, double value) { m_windowNormalIncidenceSolarEnergyTransmittance[index] = value; }

  /**
  * Window solar control factor (external control) (0 to 1).
  */
  Vector windowShadingCorrectionFactor() const { return m_windowShadingCorrectionFactor; }
  void setWindowShadingCorrectionFactor(const Vector& value) { m_windowShadingCorrectionFactor = value; }
  void setWindowShadingCorrectionFactor(int index, double value) { m_windowShadingCorrectionFactor[index] = value; }

  /**
  * Interior surface heat capacity (J/K/m2).
  */
  double interiorHeatCapacity() const { return m_interiorHeatCapacity; }
  void setInteriorHeatCapacity(double value) { m_interiorHeatCapacity = value; }

  /**
  * Exterior surface (wall) heat capacity (J/K/m2).
  */
  double wallHeatCapacity() const { return m_wallHeatCapacity; }
  void setWallHeatCapacity(double value) { m_wallHeatCapacity = value; }

  /**
  * Building height (m).
  */
  double buildingHeight() const { return m_buildingHeight; }
  void setBuildingHeight(double value) { m_buildingHeight = value; }

  /**
  * Infiltration rate occupied (m3/m2/hr, based on surface area).
  */
  double infiltrationRate() const { return m_infiltrationRate; }
  void setInfiltrationRate(double value) { m_infiltrationRate = value; }

  /**
  * External thermal surface resistance (m2*k/W).
  */
  double R_se() const { return m_R_se; }
  void setR_se(double R_se) { m_R_se = R_se; }

  /**
  * Irradiance at which moveable shading is at maximum use (W).
  */
  double irradianceForMaxShadingUse() const { return m_irradianceForMaxShadingUse; }
  void setIrradianceForMaxShadingUse(double irradianceForMaxShadingUse) { m_irradianceForMaxShadingUse = irradianceForMaxShadingUse; }

  /**
  * Shading factor at max use of moveable shading (unitless).
  */
  double shadingFactorAtMaxUse() const { return m_shadingFactorAtMaxUse; }
  void setShadingFactorAtMaxUse(double shadingFactorAtMaxUse) { m_shadingFactorAtMaxUse = shadingFactorAtMaxUse; }

  /**
  * Total interior surface area per floor area (m2/m2).
  */
  double totalAreaPerFloorArea() const { return m_totalAreaPerFloorArea; }
  void setTotalAreaPerFloorArea(double totalAreaPerFloorArea) { m_totalAreaPerFloorArea = totalAreaPerFloorArea; }

  /**
  * Window frame factor.
  */
  double win_ff() const { return m_win_ff; }
  void setWin_ff(double win_ff) { m_win_ff = win_ff; }

  /**
  * Correction factor for non-scattering window as per ISO 13790 11.4.2.
  */
  double win_F_W() const { return m_win_F_W; }
  void setWin_F_W(double win_F_W) { m_win_F_W = win_F_W; }

  /**
  * Vertical wall external convection surface heat resistance as per ISO 6946.
  */
  double R_sc_ext() const { return m_R_sc_ext; }
  void setR_sc_ext(double R_sc_ext) { m_R_sc_ext = R_sc_ext; }

private:
  // In-class initialization ensures safer default state
  double m_floorArea = 0.0;
  
  // Vectors initialized to size 9 with value 0.0
  Vector m_wallArea = Vector(9, 0.0);
  Vector m_windowArea = Vector(9, 0.0);
  Vector m_wallUniform = Vector(9, 0.0);
  Vector m_windowUniform = Vector(9, 0.0);
  Vector m_wallThermalEmissivity = Vector(9, 0.0);
  Vector m_wallSolarAbsorbtion = Vector(9, 0.0);
  Vector m_windowShadingDevice = Vector(9, 0.0);
  Vector m_windowNormalIncidenceSolarEnergyTransmittance = Vector(9, 0.0);
  Vector m_windowShadingCorrectionFactor = Vector(9, 0.0);

  double m_interiorHeatCapacity = 0.0;
  double m_wallHeatCapacity = 0.0;
  double m_buildingHeight = 0.0;
  double m_infiltrationRate = 0.0;

  // Members with default values:
  double m_R_se = 0.04; 
  double m_irradianceForMaxShadingUse = 500.0;
  double m_shadingFactorAtMaxUse = 0.5;
  double m_totalAreaPerFloorArea = 4.5;
  double m_win_ff = 0.25;
  double m_win_F_W = 0.9;
  double m_R_sc_ext = 0.04;
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_STRUCTURE_HPP