#pragma once
#include "Constants.hpp"
#include "ISOModelAPI.hpp"
#include "ISOResults.hpp"
#include "MathHelpers.hpp"
#include "Schedules.hpp"

#ifdef ISOMODEL_STANDALONE
#include "EndUses.hpp"
#else
#include "../utilities/core/Logger.hpp"
#include "../utilities/data/EndUses.hpp"
#endif

#include "Simulation.hpp"

#include <memory>

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

  struct AnnualLightingHours {
    double t_lt_D;
    double t_lt_N;
    double t_unocc;
  };

  struct PlugLoads {
    Vector v_Q_plug_elec;
    Vector v_Q_plug_gas;
  };

private:
  // Struct to hold all intermediate simulation data for MonthlyModel::simulate
  struct MonthlySimulationData {
    // From schedules::getMonthlySchedules
    schedules::MonthlyScheduleData scheduleData{};

    // From solarRadiationBreakdown
    Vector v_hrs_sun_down_mo = Vector(MONTHS_IN_YEAR);
    Vector frac_Pgh_wk_nt = Vector(MONTHS_IN_YEAR);
    Vector frac_Pgh_wke_day = Vector(MONTHS_IN_YEAR);
    Vector frac_Pgh_wke_nt = Vector(MONTHS_IN_YEAR);
    Vector v_Tdbt_nt = Vector(MONTHS_IN_YEAR);
    Vector v_Tdbt_day = Vector(MONTHS_IN_YEAR);

    // From lightingEnergyUse
    double Q_illum_occ = 0.0;
    double Q_illum_unocc = 0.0;
    double Q_illum_tot_yr = 0.0;
    Vector v_Q_illum_tot = Vector(MONTHS_IN_YEAR);
    Vector v_Q_illum_ext_tot = Vector(MONTHS_IN_YEAR);

    // From envelopCalculations
    double H_tr = 0.0;

    // From windowSolarGain
    Vector v_wall_A_sol = Vector(NUM_TOTAL_SURFACES);
    Vector v_win_hr = Vector(NUM_TOTAL_SURFACES);
    Vector v_wall_R_sc = Vector(NUM_TOTAL_SURFACES);
    Vector v_win_A_sol = Vector(NUM_TOTAL_SURFACES);

    // From solarHeatGain
    Vector v_E_sol = Vector(MONTHS_IN_YEAR);

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
    Vector v_P_tot_wke_day = Vector(MONTHS_IN_YEAR);
    Vector v_P_tot_wk_nt = Vector(MONTHS_IN_YEAR);
    Vector v_P_tot_wke_nt = Vector(MONTHS_IN_YEAR);

    // From interiorTemp
    Vector v_Th_avg = Vector(MONTHS_IN_YEAR);
    Vector v_Tc_avg = Vector(MONTHS_IN_YEAR);
    double tau = 0.0;

    // From ventilationCalc
    Vector v_Hve_ht = Vector(MONTHS_IN_YEAR);
    Vector v_Hve_cl = Vector(MONTHS_IN_YEAR);

    // From heatingAndCooling
    Vector v_Qfan_tot = Vector(MONTHS_IN_YEAR);
    Vector v_Qneed_ht = Vector(MONTHS_IN_YEAR);
    Vector v_Qneed_cl = Vector(MONTHS_IN_YEAR);
    double Qneed_ht_yr = 0.0;
    double Qneed_cl_yr = 0.0;

    // From hvac
    Vector v_Qelec_ht = Vector(MONTHS_IN_YEAR);
    Vector v_Qgas_ht = Vector(MONTHS_IN_YEAR);
    Vector v_Qcl_elec_tot = Vector(MONTHS_IN_YEAR);
    Vector v_Qcl_gas_tot = Vector(MONTHS_IN_YEAR);

    // Intermediate HVAC loads
    Vector v_Qht_sys = Vector(MONTHS_IN_YEAR);
    Vector v_Qht_DH = Vector(MONTHS_IN_YEAR);
    Vector v_Qcl_sys = Vector(MONTHS_IN_YEAR);
    Vector v_Qcool_DC = Vector(MONTHS_IN_YEAR);

    // From pump
    Vector v_Q_pump_tot = Vector(MONTHS_IN_YEAR);

    // From calculateAirVolumes
    Vector v_Vair_ht = Vector(MONTHS_IN_YEAR);
    Vector v_Vair_cl = Vector(MONTHS_IN_YEAR);

    // From calculateTotalAirFlow
    Vector v_Vair_tot = Vector(MONTHS_IN_YEAR);

    // From heatedWater
    Vector v_Q_dhw_elec = Vector(MONTHS_IN_YEAR);
    Vector v_Q_dhw_gas = Vector(MONTHS_IN_YEAR);
  };

private:
  // Simulation functions.
  void solarRadiationBreakdown(MonthlySimulationData &simData) const;
  void lightingEnergyUse(MonthlySimulationData &simData) const;
  void envelopeCalculations(MonthlySimulationData &simData) const;
  void windowSolarGain(MonthlySimulationData &simData) const;
  void solarHeatGain(MonthlySimulationData &simData) const;
  void calculateInternalGainComponents(MonthlySimulationData &simData) const;
  void unoccupiedHeatGain(MonthlySimulationData &simData) const;
  void calculateInteriorTemperatures(MonthlySimulationData &simData) const;

  void calculateVentilation(MonthlySimulationData &simData) const;
  void calculateHeatingAndCoolingNeeds(MonthlySimulationData &simData) const;
  void calculateHVACEnergyUse(MonthlySimulationData &simData) const;
  void calculatePumpEnergy(MonthlySimulationData &simData) const;

  // Helper for pump energy calculation
  static Vector calculatePumpEnergyForMode(const Vector &v_Qneed_mode, const Vector &v_Qneed_total,
                                           double E_pumps_w_per_m2, double pump_control_reduction,
                                           double floor_area);

  // Helper for lighting energy use
  static AnnualLightingHours calculateAnnualLightingOperationalHours(const Lighting &lights,
                                                                     const Population &pop);

  void energyGeneration() const;

  void calculateHeatedWaterEnergy(MonthlySimulationData &simData) const;

  // Helper for solarHeatGain
  static Matrix buildSolarIrradianceMatrix(const WeatherData &weather);

  std::vector<EndUses> outputGeneration(const MonthlySimulationData &simData) const;

#ifdef _OPENSTUDIOS
  REGISTER_LOGGER("openstudio.isomodel.MonthlyModel");
#endif
};
} // namespace openstudio::isomodel

