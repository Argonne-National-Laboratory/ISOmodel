/**********************************************************************
 *  Copyright (c) 2008-2015, Alliance for Sustainable Energy.
 *  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *USA
 **********************************************************************/

#ifndef ISOMODEL_MONTHLYMODEL_HPP
#define ISOMODEL_MONTHLYMODEL_HPP

#include "ISOModelAPI.hpp"
#include "ISOResults.hpp"
#include "MathHelpers.hpp"
#include "Schedules.hpp"
#include "Constants.hpp"

#ifdef ISOMODEL_STANDALONE
#include "EndUses.hpp"
#else
#include "../utilities/core/Logger.hpp"
#include "../utilities/data/EndUses.hpp"
#endif

#include <memory>

#include "Simulation.hpp"

namespace openstudio::isomodel {

class EndUses;
class ISOMODEL_API MonthlyModel : public Simulation {
public:
  /**
   * Creates an empty MonthlyModel. Generally, the MonthlyModel should be
   * created using the UserModel::toMonthlyModel() method.
   */
  MonthlyModel();
  ~MonthlyModel() override;

  /**
   * Runs the ISO Model cacluations using the ISO 13790 monthly method for the
   * given set of input parameters. returns a vector of EndUses, one EndUses per
   * month of the year
   */
  std::vector<EndUses> simulate() const;

private:
  // Struct to hold all intermediate simulation data for MonthlyModel::simulate
  struct MonthlySimulationData {
    // From schedules::getMonthlySchedules
    schedules::MonthlyScheduleData scheduleData{};

    // From solarRadiationBreakdown
    Vector v_hrs_sun_down_mo = Vector(monthsInYear);
    Vector frac_Pgh_wk_nt = Vector(monthsInYear);
    Vector frac_Pgh_wke_day = Vector(monthsInYear);
    Vector frac_Pgh_wke_nt = Vector(monthsInYear);
    Vector v_Tdbt_nt = Vector(monthsInYear);
    Vector v_Tdbt_day = Vector(monthsInYear);

    // From lightingEnergyUse
    double Q_illum_occ = 0.0;
    double Q_illum_unocc = 0.0;
    double Q_illum_tot_yr = 0.0;
    Vector v_Q_illum_tot = Vector(monthsInYear);
    Vector v_Q_illum_ext_tot = Vector(monthsInYear);

    // From envelopCalculations
    Vector v_win_A = Vector(numTotalSurfaces);
    Vector v_wall_emiss = Vector(numTotalSurfaces);
    Vector v_wall_alpha_sc = Vector(numTotalSurfaces);
    Vector v_wall_U = Vector(numTotalSurfaces);
    Vector v_wall_A = Vector(numTotalSurfaces);
    double H_tr = 0.0;

    // From windowSolarGain
    Vector v_wall_A_sol = Vector(numTotalSurfaces);
    Vector v_win_hr = Vector(numTotalSurfaces);
    Vector v_wall_R_sc = Vector(numTotalSurfaces);
    Vector v_win_A_sol = Vector(numTotalSurfaces);

    // From solarHeatGain
    Vector v_E_sol = Vector(monthsInYear);

    // From heatGainsAndLosses
    double phi_int_avg = 0.0;
    double phi_plug_avg = 0.0;
    double phi_illum_avg = 0.0;
    double phi_int_wke_nt = 0.0;
    double phi_int_wke_day = 0.0;
    double phi_int_wk_nt = 0.0;

    // From internalHeatGain
    double phi_I_tot = 0.0;

    // From unoccupiedHeatGain
    Vector v_P_tot_wke_day = Vector(monthsInYear);
    Vector v_P_tot_wk_nt = Vector(monthsInYear);
    Vector v_P_tot_wke_nt = Vector(monthsInYear);

    // From interiorTemp
    Vector v_Th_avg = Vector(monthsInYear);
    Vector v_Tc_avg = Vector(monthsInYear);
    double tau = 0.0;

    // From ventilationCalc
    Vector v_Hve_ht = Vector(monthsInYear);
    Vector v_Hve_cl = Vector(monthsInYear);

    // From heatingAndCooling
    Vector v_Qfan_tot = Vector(monthsInYear);
    Vector v_Qneed_ht = Vector(monthsInYear);
    Vector v_Qneed_cl = Vector(monthsInYear);
    double Qneed_ht_yr = 0.0;
    double Qneed_cl_yr = 0.0;

    // From hvac
    Vector v_Qelec_ht = Vector(monthsInYear);
    Vector v_Qgas_ht = Vector(monthsInYear);
    Vector v_Qcl_elec_tot = Vector(monthsInYear);
    Vector v_Qcl_gas_tot = Vector(monthsInYear);

    // Intermediate HVAC loads
    Vector v_Qht_sys = Vector(monthsInYear);
    Vector v_Qht_DH = Vector(monthsInYear);
    Vector v_Qcl_sys = Vector(monthsInYear);
    Vector v_Qcool_DC = Vector(monthsInYear);

    // From pump
    Vector v_Q_pump_tot = Vector(monthsInYear);

    // From calculateAirVolumes
    Vector v_Vair_ht = Vector(monthsInYear);
    Vector v_Vair_cl = Vector(monthsInYear);

    // From calculateTotalAirFlow
    Vector v_Vair_tot = Vector(monthsInYear);

    // From heatedWater
    Vector v_Q_dhw_elec = Vector(monthsInYear);
    Vector v_Q_dhw_gas = Vector(monthsInYear);
  };

private:
  // Simulation functions.
  void solarRadiationBreakdown(MonthlySimulationData &simData) const;
  void lightingEnergyUse(MonthlySimulationData &simData) const;
  void envelopeCalculations(MonthlySimulationData &simData) const;
  void windowSolarGain(MonthlySimulationData &simData) const;
  void solarHeatGain(MonthlySimulationData &simData) const;
  void calculateInternalGainComponents(MonthlySimulationData &simData) const;
  void calculateTotalInternalGain(MonthlySimulationData &simData) const;
  void unoccupiedHeatGain(MonthlySimulationData &simData) const;
  void calculateInteriorTemperatures(MonthlySimulationData &simData) const;
  static double calculateBEMAdjustment(const Building& building);

  void calculateWeekendTemperatures(
      const Vector &v_decay_start_base, const Vector &v_limit_start_col0,
      double tset_unocc, double tau, const Vector &v_ti, const Matrix &M_dT,
      const Matrix &M_Te, Vector &v_wke_avg, Vector &v_wk_nt) const;

  // Helper struct for lighting operational hours
  struct WindowShadingComponents {
    Vector v_win_ff;
    Vector v_win_F_shgl;
  };
  static WindowShadingComponents calculateWindowShadingComponents(const Structure& structure);

  struct AnnualLightingHours {
    double t_lt_D;
    double t_lt_N;
    double t_unocc;
  };
  void calculateSunHours(const Matrix &m_mhEgh,
                         Vector &v_hrs_sun_down_mo) const;

  void calculateVentilation(MonthlySimulationData &simData) const;
  void calculateHeatingAndCoolingNeeds(MonthlySimulationData &simData) const;
  void calculateHVACEnergyUse(MonthlySimulationData &simData) const;
  // Helper functions for hvac
  void calculateHeatingSystemLoads(MonthlySimulationData &simData, const Vector &v_Qloss_ht_dist) const;
  void calculateCoolingSystemLoads(MonthlySimulationData &simData, const Vector &v_Qloss_cl_dist, double IEER) const;
  void calculatePumpEnergy(MonthlySimulationData &simData) const;

  // Helper for pump energy calculation
  static Vector calculatePumpEnergyForMode(const Vector &v_Qneed_mode,
                                           const Vector &v_Qneed_total,
                                           double E_pumps_w_per_m2,
                                           double pump_control_reduction,
                                           double floor_area);

  // Helper for lighting energy use
  AnnualLightingHours calculateAnnualLightingOperationalHours() const;

  void energyGeneration() const;

  void calculateHeatedWaterEnergy(MonthlySimulationData &simData) const;

  // Helper for solarHeatGain
  Vector calculateUtilizationFactor(const Vector &gamma_H, double a_H) const;
  void calculateAirVolumes(MonthlySimulationData &simData) const;
  void calculateTotalAirFlow(MonthlySimulationData &simData) const;
  void calculateFanEnergy(MonthlySimulationData &simData) const;
  static Matrix buildSolarIrradianceMatrix(const WeatherData& weather);
  static Vector calculateGlazingSolarHeatGain(const Matrix &m_I_sol, const Vector &v_win_A_sol, const Structure& structure);
  static Vector calculateOpaqueSolarHeatGain(const Matrix &m_I_sol, const Vector &v_wall_A_sol, const Vector &v_wall_phi_r);

  // Helper for heatGainsAndLosses
  static double calculatePeopleHeatGain(const Population &pop, bool occupied);
  static double calculateApplianceHeatGain(const Building &building, bool occupied);
  static double calculateIlluminationHeatGain(double Q_illum_val, double hours_fraction, double floor_area);
  static double calculateAverageIlluminationHeatGain(double Q_illum_tot_yr, double floor_area);

  // Helper for unoccupiedHeatGain
  static Vector calculatePeriodHeatGain(double phi_int_period,
                                        const Vector &megaseconds_period,
                                        const Vector &frac_Pgh_period,
                                        const Vector &v_E_sol, double floor_area);

  // Helper for outputGeneration
  struct PlugLoads {
    Vector v_Q_plug_elec;
    Vector v_Q_plug_gas;
  };
  static PlugLoads calculatePlugLoads(const Building& building, double frac_hrs_wk_day);
  static Vector convertEnergyToKWhPerSqM(const Vector &energy_MJ, double floor_area);

  std::vector<EndUses> outputGeneration(const MonthlySimulationData &simData) const;

#ifdef _OPENSTUDIOS
  REGISTER_LOGGER("openstudio.isomodel.MonthlyModel");
#endif
};
} // namespace openstudio::isomodel

#endif // ISOMODEL_MONTHLYMODEL_HPP
