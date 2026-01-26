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
  // Simulation functions.
  void solarRadiationBreakdown(const Vector &weekdayOccupiedMegaseconds,
                               const Vector &weekdayUnoccupiedMegaseconds,
                               const Vector &weekendOccupiedMegaseconds,
                               const Vector &weekendUnoccupiedMegaseconds,
                               const Vector &clockHourOccupied,
                               const Vector &clockHourUnoccupied,
                               Vector &v_hrs_sun_down_mo,
                               Vector &frac_Pgh_wk_nt, Vector &frac_Pgh_wke_day,
                               Vector &frac_Pgh_wke_nt, Vector &v_Tdbt_nt,
                               Vector &v_Tdbt_Day) const;
  void lightingEnergyUse(const Vector &v_hrs_sun_down_mo, double &Q_illum_occ,
                         double &Q_illum_unocc, double &Q_illum_tot_yr,
                         Vector &v_Q_illum_tot,
                         Vector &v_Q_illum_ext_tot) const;

  void envelopCalculations(Vector &v_win_A, Vector &v_wall_emiss,
                           Vector &v_wall_alpha_sc, Vector &v_wall_U,
                           Vector &v_wall_A, double &H_tr) const;

  void windowSolarGain(const Vector &v_win_A, const Vector &v_wall_emiss,
                       const Vector &v_wall_alpha_sc, const Vector &v_wall_U,
                       const Vector &v_wall_A, Vector &v_wall_A_sol,
                       Vector &v_win_hr, Vector &v_wall_R_sc,
                       Vector &v_win_A_sol) const;

  void solarHeatGain(const Vector &v_win_A_sol, const Vector &v_wall_R_sc,
                     const Vector &v_wall_U, const Vector &v_wall_A,
                     const Vector &v_win_hr, const Vector &v_wall_A_sol,
                     Vector &v_E_sol) const;

  void heatGainsAndLosses(double frac_hrs_wk_day, double Q_illum_occ,
                          double Q_illum_unocc, double Q_illum_tot_yr,
                          double &phi_int_avg, double &phi_plug_avg,
                          double &phi_illum_avg, double &phi_int_wke_nt,
                          double &phi_int_wke_day, double &phi_int_wk_nt) const;

  void internalHeatGain(double phi_int_avg, double phi_plug_avg,
                        double phi_illum_avg, double &phi_I_tot) const;

  void unoccupiedHeatGain(double phi_int_wk_nt, double phi_int_wke_day,
                          double phi_int_wke_nt,
                          const Vector &weekdayUnoccupiedMegaseconds,
                          const Vector &weekendOccupiedMegaseconds,
                          const Vector &weekendUnoccupiedMegaseconds,
                          const Vector &frac_Pgh_wk_nt,
                          const Vector &frac_Pgh_wke_day,
                          const Vector &frac_Pgh_wke_nt, const Vector &v_E_sol,
                          Vector &v_P_tot_wke_day, Vector &v_P_tot_wk_nt,
                          Vector &v_P_tot_wke_nt) const;

  void interiorTemp(const Vector &v_wall_A, const Vector &v_P_tot_wke_day,
                    const Vector &v_P_tot_wk_nt, const Vector &v_P_tot_wke_nt,
                    const Vector &v_Tdbt_nt, const Vector &v_Tdbt_day,
                    double H_tr, double hoursUnoccupiedPerDay,
                    double hoursOccupiedPerDay, double frac_hrs_wk_day,
                    double frac_hrs_wk_nt, double frac_hrs_wke_tot,
                    Vector &v_Th_avg, Vector &v_Tc_avg, double &tau) const;

  double calculateBEMAdjustment() const;

  void calculateWeekendTemperatures(
      const Vector &v_decay_start_base, const Vector &v_limit_start_col0,
      double tset_unocc, double tau, const Vector &v_ti, const Matrix &M_dT,
      const Matrix &M_Te, Vector &v_wke_avg, Vector &v_wk_nt) const;

  // Helper struct for lighting operational hours
  struct WindowShadingComponents {
    Vector v_win_ff;
    Vector v_win_F_shgl;
  };
  WindowShadingComponents calculateWindowShadingComponents() const;

  struct AnnualLightingHours {
    double t_lt_D;
    double t_lt_N;
    double t_unocc;
  };
  void calculateSunHours(const Matrix &m_mhEgh,
                         Vector &v_hrs_sun_down_mo) const;

  void ventilationCalc(const Vector &v_Th_avg, const Vector &v_Tc_avg,
                       double frac_hrs_wk_day, Vector &v_Hve_ht,
                       Vector &v_Hve_cl) const;

  void heatingAndCooling(const Vector &v_E_sol, const Vector &v_Th_avg,
                         const Vector &v_Hve_ht, const Vector &v_Tc_avg,
                         const Vector &v_Hve_cl, double tau, double H_tr,
                         double phi_I_tot, double frac_hrs_wk_day,
                         Vector &v_Qfan_tot, Vector &v_Qneed_ht,
                         Vector &v_Qneed_cl, double &Qneed_ht_yr,
                         double &Qneed_cl_yr) const;

  void hvac(const Vector &v_Qneed_ht, const Vector &v_Qneed_cl,
            double Qneed_ht_yr, double Qneed_cl_yr, Vector &v_Qelec_ht,
            Vector &v_Qgas_ht, Vector &v_Qcl_elec_tot,
            Vector &v_Qcl_gas_tot) const; // Original function signature

  // Helper functions for hvac
  void calculateHeatingSystemLoads(const Vector &v_Qneed_ht, const Vector &v_Qloss_ht_dist,
                                   Vector &v_Qht_sys, Vector &v_Qht_DH) const;
  void calculateCoolingSystemLoads(const Vector &v_Qneed_cl, const Vector &v_Qloss_cl_dist,
                                   double IEER, Vector &v_Qcl_sys, Vector &v_Qcool_DC) const;
  void pump(const Vector &v_Qneed_ht, const Vector &v_Qneed_cl,
            double Qneed_ht_yr, double Qneed_cl_yr, Vector &v_Q_pump_tot) const;

  // Helper for lighting energy use
  AnnualLightingHours calculateAnnualLightingOperationalHours() const;

  void energyGeneration() const;

  void heatedWater(Vector &v_Q_dhw_elec, Vector &v_Q_dhw_gas) const;

  // Helper for solarHeatGain
  Vector calculateUtilizationFactor(const Vector &gamma_H, double a_H) const;
  std::pair<Vector, Vector>
  calculateAirVolumes(const Vector &Qneed_ht, const Vector &Qneed_cl,
                      const Vector &Th_avg, const Vector &Tc_avg) const;
  Vector calculateTotalAirFlow(const Vector &v_Vair_ht,
                               const Vector &v_Vair_cl,
                               double frac_hrs_wk_day) const;
  Vector calculateFanEnergy(const Vector &Vair_tot) const;
  Matrix buildSolarIrradianceMatrix() const;
  Vector calculateGlazingSolarHeatGain(const Matrix &m_I_sol,
                                       const Vector &v_win_A_sol) const;
  Vector calculateOpaqueSolarHeatGain(const Matrix &m_I_sol,
                                      const Vector &v_wall_A_sol,
                                      const Vector &v_wall_phi_r) const;

  // Helper for heatGainsAndLosses
  double calculatePeopleHeatGain(bool occupied) const;
  double calculateApplianceHeatGain(bool occupied) const;
  double calculateIlluminationHeatGain(double Q_illum_val, double hours_fraction) const;
  double calculateAverageIlluminationHeatGain(double Q_illum_tot_yr) const;

  // Helper for unoccupiedHeatGain
  Vector calculatePeriodHeatGain(double phi_int_period,
                                 const Vector &megaseconds_period,
                                 const Vector &frac_Pgh_period,
                                 const Vector &v_E_sol) const;

  // Helper for outputGeneration
  struct PlugLoads {
    Vector v_Q_plug_elec;
    Vector v_Q_plug_gas;
  };
  PlugLoads calculatePlugLoads(double frac_hrs_wk_day) const;
  Vector convertEnergyToKWhPerSqM(const Vector &energy_MJ, double floor_area) const;

  std::vector<EndUses>
  outputGeneration(const Vector &v_Qelec_ht, const Vector &v_Qcl_elec_tot,
                   const Vector &v_Q_illum_tot, const Vector &v_Q_illum_ext_tot,
                   const Vector &v_Qfan_tot, const Vector &v_Q_pump_tot,
                   const Vector &v_Q_dhw_elec, const Vector &v_Qgas_ht,
                   const Vector &v_Qcl_gas_tot, const Vector &v_Q_dhw_gas,
                   double frac_hrs_wk_day) const;

#ifdef _OPENSTUDIOS
  REGISTER_LOGGER("openstudio.isomodel.MonthlyModel");
#endif
};
} // namespace openstudio::isomodel

#endif // ISOMODEL_MONTHLYMODEL_HPP
