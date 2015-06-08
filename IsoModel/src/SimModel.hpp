/**********************************************************************
 *  Copyright (c) 2008-2013, Alliance for Sustainable Energy.
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/

#ifndef ISOMODEL_SIMMODEL_HPP
#define ISOMODEL_SIMMODEL_HPP

#include "ISOModelAPI.hpp"
#ifdef _OPENSTUDIO
#include <utilities/core/Logger.hpp>
#endif
#include <memory>

#include "Simulation.hpp"

#include "EndUses.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"

namespace openstudio {

class EndUses;

namespace isomodel {
#ifndef DBL_MAX
#define DBL_MAX    1.7976931348623157E+308
#endif
#ifndef DBL_MIN
#define DBL_MIN    2.2250738585072014E-308
#endif

//flag to turn on debug printing of many intermediate variables to stdout
#define DEBUG_ISO_MODEL_SIMULATION false
typedef unsigned int uint;

ISOMODEL_API void printVector(const char* vecName, Vector vec);
ISOMODEL_API void printMatrix(const char* matName, Matrix mat);
void printMatrix(const char* matName, double* mat, unsigned int dim1, unsigned int dim2);

ISOMODEL_API Vector mult(const double* v1, const double s1, int size);
ISOMODEL_API Vector mult(const Vector& v1, const double s1);
ISOMODEL_API Vector mult(const Vector& v1, const double* v2);
ISOMODEL_API Vector mult(const Vector& v1, const Vector& v2);
ISOMODEL_API Vector div(const Vector& v1, const double s1);
ISOMODEL_API Vector div(const double s1, const Vector& v1);
ISOMODEL_API Vector div(const Vector& v1, const Vector& v2);
ISOMODEL_API Vector sum(const Vector& v1, const Vector& v2);
ISOMODEL_API Vector sum(const Vector& v1, const double v2);
ISOMODEL_API Vector dif(const Vector& v1, const Vector& v2);
ISOMODEL_API Vector dif(const Vector& v1, const double v2);
ISOMODEL_API Vector dif(const double v1, const Vector& v2);

ISOMODEL_API Vector maximum(const Vector& v1, const Vector& v2);
ISOMODEL_API Vector maximum(const Vector& v1, double val);
ISOMODEL_API double maximum(const Vector& v1);

ISOMODEL_API Vector minimum(const Vector& v1, double val);
ISOMODEL_API double minimum(const Vector& v1);

ISOMODEL_API Vector abs(const Vector& v1);
ISOMODEL_API Vector pow(const Vector& v1, const double xp);

struct ISOMODEL_API ISOResults
{
  std::vector<EndUses> monthlyResults;
  std::vector<EndUses> hourlyResults;
};

class ISOMODEL_API SimModel : public Simulation
{
public:
  SimModel();
  virtual ~SimModel();

  /*
   *  Runs the ISO Model cacluations for the given set of input parameters.
   *  returns ISOResults which is a vector of EndUses, one EndUses per month of the year
   */
  ISOResults simulate() const;

private:
  // Simulation functions.
  void scheduleAndOccupancy(Vector& weekdayOccupiedMegaseconds, Vector& weekdayUnoccupiedMegaseconds, Vector& weekendOccupiedMegaseconds,
      Vector& weekendUnoccupiedMegaseconds, Vector& clockHourOccupied, Vector& clockHourUnoccupied, double& frac_hrs_wk_day,
      double& hoursUnoccupiedPerDay, double& hoursOccupiedPerDay, double& frac_hrs_wk_nt, double& frac_hrs_wke_tot) const;

  void solarRadiationBreakdown(const Vector& weekdayOccupiedMegaseconds, const Vector& weekdayUnoccupiedMegaseconds,
      const Vector& weekendOccupiedMegaseconds, const Vector& weekendUnoccupiedMegaseconds, const Vector& clockHourOccupied,
      const Vector& clockHourUnoccupied, Vector& v_hrs_sun_down_mo, Vector& frac_Pgh_wk_nt, Vector& frac_Pgh_wke_day, Vector& frac_Pgh_wke_nt,
      Vector& v_Tdbt_nt, Vector& v_Tdbt_Day) const;
  void lightingEnergyUse(const Vector& v_hrs_sun_down_mo, double& Q_illum_occ, double& Q_illum_unocc, double& Q_illum_tot_yr, Vector& v_Q_illum_tot,
      Vector& v_Q_illum_ext_tot) const;

  void envelopCalculations(Vector& v_win_A, Vector& v_wall_emiss, Vector& v_wall_alpha_sc, Vector& v_wall_U, Vector& v_wall_A, double& H_tr) const;

  void windowSolarGain(const Vector& v_win_A, const Vector& v_wall_emiss, const Vector& v_wall_alpha_sc, const Vector& v_wall_U, const Vector& v_wall_A,
      Vector& v_wall_A_sol, Vector& v_win_hr, Vector& v_wall_R_sc, Vector& v_win_A_sol) const;

  void solarHeatGain(const Vector& v_win_A_sol, const Vector& v_wall_R_sc, const Vector& v_wall_U, const Vector& v_wall_A, const Vector& v_win_hr,
      const Vector& v_wall_A_sol, Vector& v_E_sol) const;

  void heatGainsAndLosses(double frac_hrs_wk_day, double Q_illum_occ, double Q_illum_unocc, double Q_illum_tot_yr, double& phi_int_avg,
      double& phi_plug_avg, double& phi_illum_avg, double& phi_int_wke_nt, double& phi_int_wke_day, double& phi_int_wk_nt) const;

  void internalHeatGain(double phi_int_avg, double phi_plug_avg, double phi_illum_avg, double& phi_I_tot) const;

  void unoccupiedHeatGain(double phi_int_wk_nt, double phi_int_wke_day, double phi_int_wke_nt, const Vector& weekdayUnoccupiedMegaseconds,
      const Vector& weekendOccupiedMegaseconds, const Vector& weekendUnoccupiedMegaseconds, const Vector& frac_Pgh_wk_nt,
      const Vector& frac_Pgh_wke_day, const Vector& frac_Pgh_wke_nt, const Vector& v_E_sol, Vector& v_P_tot_wke_day, Vector& v_P_tot_wk_nt,
      Vector& v_P_tot_wke_nt) const;
  
  void interiorTemp(const Vector& v_wall_A, const Vector& v_P_tot_wke_day, const Vector& v_P_tot_wk_nt, const Vector& v_P_tot_wke_nt,
      const Vector& v_Tdbt_nt, const Vector& v_Tdbt_day, double H_tr, double hoursUnoccupiedPerDay, double hoursOccupiedPerDay, double frac_hrs_wk_day, double frac_hrs_wk_nt,
      double frac_hrs_wke_tot, Vector& v_Th_avg, Vector& v_Tc_avg, double& tau) const;

  void ventilationCalc(const Vector& v_Th_avg, const Vector& v_Tc_avg, double frac_hrs_wk_day, Vector& v_Hve_ht, Vector& v_Hve_cl) const;

  void heatingAndCooling(const Vector& v_E_sol, const Vector& v_Th_avg, const Vector& v_Hve_ht, const Vector& v_Tc_avg, const Vector& v_Hve_cl, double tau,
      double H_tr, double phi_I_tot, double frac_hrs_wk_day, Vector& v_Qfan_tot, Vector& v_Qneed_ht, Vector& v_Qneed_cl, double& Qneed_ht_yr,
      double& Qneed_cl_yr) const;

  void hvac(const Vector& v_Qneed_ht, const Vector& v_Qneed_cl, double Qneed_ht_yr, double Qneed_cl_yr, Vector& v_Qelec_ht, Vector& v_Qgas_ht,
      Vector& v_Qcl_elec_tot, Vector& v_Qcl_gas_tot) const;
  void pump(const Vector& v_Qneed_ht, const Vector& v_Qneed_cl, double Qneed_ht_yr, double Qneed_cl_yr, Vector& v_Q_pump_tot) const;

  void energyGeneration() const;

  void heatedWater(Vector& v_Q_dhw_elec, Vector& v_Q_dhw_gas) const;

  ISOResults outputGeneration(const Vector& v_Qelec_ht, const Vector& v_Qcl_elec_tot, const Vector& v_Q_illum_tot, const Vector& v_Q_illum_ext_tot,
      const Vector& v_Qfan_tot, const Vector& v_Q_pump_tot, const Vector& v_Q_dhw_elec, const Vector& v_Qgas_ht, const Vector& v_Qcl_gas_tot,
      const Vector& v_Q_dhw_gas, double frac_hrs_wk_day) const;

  // Internal member variables for various defaults and constants in the simulation calcs.

  // Window solar gain constants.
  // Window frame factor.
  double n_win_ff;

  // Correction factor for non-scattering window as per ISO 13790 11.4.2
  double n_win_F_W;

  // Vertical wall external convection surface heat resistance as per ISO 6946
  double n_R_sc_ext;

  // Interior temp constants.

  // Flag to signify if we have heating and controls turned on or off. E.g., might be off for school in summer.
  double T_ht_ctrl_flag;

  // Flag to signify if we have cooling and controls turned on or off. E.g., might be off for school in summer.
  double T_cl_ctrl_flag;

  // Overall heat transfer coefficient by ventilation as per ISO 13790 9.3.
  // TODO: This is not implemented yet.
  double H_ve;

  // Ventillation constants.
  // infiltration data from
  // Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
  // Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
  // Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.
  // create a different table for different building types
  // n_highrise_inf_table=[4 6 10 15 20];  % infiltration table for high rise buildings as per Tamura, Orm and Emmerich

  // Assumed floor exponent for infiltration pressure conversion.
  double n_p_exp;

  // Fraction that h_stack/zone height.  assume 0.7 as per en 15242
  double n_zone_frac;

  // Reset the pressure exponent to 0.667 for this part of the calc
  double n_stack_exp;

  double n_stack_coeff;
  double n_wind_exp;
  double n_wind_coeff;

  // conventional value for cp difference between windward and leeward sides for low rise buildings as per 15242.
  double n_dCp;

  // vent_rate_flag set to 0 for constant ventilation, 1 if vent of in unoccupied times or 2 if vent rate dropped proportional to population.
  int vent_rate_flag;

  // Heat capacity of air per unit volume in J/(m3 K)
  double n_rhoc_air;

  // Heating and cooling constants.

  // Reference dimensionless parameter. (Used to set a_H, the building heating dimensionless constant).
  double a_H0;

  // Reference time constant. (Used to set a_H, the building heating dimensionless constant).
  double tau_H0;

  // Heating temp diff between supply air and room air.
  double n_dT_supp_ht;

  // Cooling temp diff between supply air and room air.
  double n_dT_supp_cl;

  // rho*Cp for air (MJ/m3/K)
  double n_rhoC_a;

  // HVAC constants
  // District heating and cooling constants.
  // Building connected to DH (0=no, 1=yes.  Assume DH is powered by natural gas).
  double DH_YesNo;
  // Efficiency of DH network. Typical value 0l75-0l9 EN 15316-4-5
  double n_eta_DH_network;
  // Efficiency of DH system.
  double n_eta_DH_sys;
  // Fraction fo free heat source to DH (0 to 1).
  double n_frac_DH_free;

  // Building connected to DC (0 = no, 1 = yes).
  double DC_YesNo;
  // Efficiency of DC network
  double n_eta_DC_network;
  // COP of DC elec chillers.
  double n_eta_DC_COP;
  // Fraction of DC chillers that are absorption
  double n_eta_DC_frac_abs;
  // COP of DC absorption chillers.
  double n_eta_DC_COP_abs;
  // Fraction of free heat source to absorption DC chillers (0 to 1).
  double n_frac_DC_free;

  // Pump constants
  // Specific power of systems pumps + control systems in W/m2.
  double n_E_pumps;

  // Heated water constants.
  // Water temp set point (C).
  double n_dhw_tset;
  // Water initial temp (C).
  double n_dhw_tsupply;
  // Specific heat of water in MJ/m3/K.
  double n_CP_h20;


#ifdef _OPENSTUDIOS
  REGISTER_LOGGER("openstudio.isomodel.SimModel");
#endif
};
} // isomodel
} // openstudio

#endif // ISOMODEL_SIMMODEL_HPP
