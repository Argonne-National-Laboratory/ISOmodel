/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/
#include "MonthlyModel.hpp"
 //to run main
#include "UserModel.hpp"
// [Refactor] Include the helpers for vector/matrix math
#include "MonthlyModelHelpers.hpp"

#include <cmath>
#include <algorithm>
#include <cfloat>
#include <iostream>

namespace openstudio {
    namespace isomodel {

        // Constants
        const double daysInMonth[] =
        { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        const double hoursInMonth[] =
        { 744, 672, 744, 720, 744, 720, 744, 744, 720, 744, 720, 744 };
        const double megasecondsInMonth[] =
        { 2.6784, 2.4192, 2.6784, 2.592, 2.6784, 2.592, 2.6784, 2.6784, 2.592, 2.6784, 2.592, 2.6784 };
        const double monthFractionOfYear[] =
        { 0.0849315068493151, 0.0767123287671233, 0.0849315068493151, 0.0821917808219178, 0.0849315068493151, 0.0821917808219178, 0.0849315068493151,
            0.0849315068493151, 0.0821917808219178, 0.0849315068493151, 0.0821917808219178, 0.0849315068493151 };
        const double daysInYear = 365;
        const double hoursInYear = 8760;
        const double hoursInWeek = 168;
        const double EECALC_NUM_MONTHS = 12;
        const double EECALC_NUM_HOURS = 24;
        const double EECALC_WEEKDAY_START = 7;
        const double kWh2MJ = 3.6f;

        // TODO: All member variables initialized in the constructor should eventually be initialized
        // by the .ism file or a default initialization of some sort.
        MonthlyModel::MonthlyModel() {}
        MonthlyModel::~MonthlyModel() {}

        //Solver functions
        void MonthlyModel::scheduleAndOccupancy(Vector& weekdayOccupiedMegaseconds, Vector& weekdayUnoccupiedMegaseconds, Vector& weekendOccupiedMegaseconds,
            Vector& weekendUnoccupiedMegaseconds, Vector& clockHourOccupied, Vector& clockHourUnoccupied, double& frac_hrs_wk_day,
            double& hoursUnoccupiedPerDay, double& hoursOccupiedPerDay, double& frac_hrs_wk_nt, double& frac_hrs_wke_tot) const
        {
            hoursOccupiedPerDay = pop.hoursEnd() - pop.hoursStart();
            if (hoursOccupiedPerDay < 0) {
                hoursOccupiedPerDay += 24;
            }
            double daysOccupiedPerWeek = pop.daysEnd() - pop.daysStart() + 1;
            if (daysOccupiedPerWeek < 0) {
                daysOccupiedPerWeek += 7;
            }

            double hoursOccupiedDuringWeek = hoursOccupiedPerDay * daysOccupiedPerWeek;
            frac_hrs_wk_day = hoursOccupiedDuringWeek / hoursInWeek;

            hoursUnoccupiedPerDay = 24 - hoursOccupiedPerDay;
            double hoursUnoccupiedDuringWeek = (daysOccupiedPerWeek - 1) * hoursUnoccupiedPerDay;
            frac_hrs_wk_nt = hoursUnoccupiedDuringWeek / hoursInWeek;

            double occupationDensity = pop.densityOccupied();
            double unoccupiedDensity = pop.densityUnoccupied();
            double densityRatio = occupationDensity / unoccupiedDensity;

            double totalWeekendHours = hoursInWeek - hoursOccupiedDuringWeek - hoursUnoccupiedDuringWeek;
            frac_hrs_wke_tot = totalWeekendHours / hoursInWeek;

            double weekendHoursOccupied = (7 - daysOccupiedPerWeek) * hoursOccupiedPerDay;
            double frac_hrs_wke_day = weekendHoursOccupied / hoursInWeek;

            double weekendHoursUnoccupied = totalWeekendHours - weekendHoursOccupied;
            double frac_hrs_wke_nt = weekendHoursUnoccupied / hoursInWeek;

            for (int m = 0; m < EECALC_NUM_MONTHS; m++) {
                weekdayOccupiedMegaseconds[m] = megasecondsInMonth[m] * frac_hrs_wk_day;
                weekdayUnoccupiedMegaseconds[m] = megasecondsInMonth[m] * frac_hrs_wk_nt;
                weekendOccupiedMegaseconds[m] = megasecondsInMonth[m] * frac_hrs_wke_day;
                weekendUnoccupiedMegaseconds[m] = megasecondsInMonth[m] * frac_hrs_wke_nt;
            }
            for (int h = 0; h < EECALC_NUM_HOURS; h++) {
                if (h - EECALC_WEEKDAY_START >= 0 && h - EECALC_WEEKDAY_START < hoursOccupiedPerDay) {
                    clockHourOccupied[h] = 1;
                    clockHourUnoccupied[h] = 0;
                }
                else {
                    clockHourOccupied[h] = 0;
                    clockHourUnoccupied[h] = 1;
                }
            }
        }

        /**
         * Breaks down the solar radiation and temperature data into day, night,
         * weekday and weekend vectors, as appropriate.
         */
        void MonthlyModel::solarRadiationBreakdown(const Vector& weekdayOccupiedMegaseconds, const Vector& weekdayUnoccupiedMegaseconds,
            const Vector& weekendOccupiedMegaseconds, const Vector& weekendUnoccupiedMegaseconds, const Vector& clockHourOccupied,
            const Vector& clockHourUnoccupied, Vector& v_hrs_sun_down_mo, Vector& frac_Pgh_wk_nt, Vector& frac_Pgh_wke_day, Vector& frac_Pgh_wke_nt,
            Vector& v_Tdbt_nt, Vector& v_Tdbt_Day) const
        {
            // Copy to a new variables so matrix nature is clear.
            Matrix m_mhEgh = location.weather()->mhEgh();
            Matrix m_mhdbt = location.weather()->mhdbt();

            // Note, these are matrix multiplies (matrix*vector) resulting in a vector.

            // monthly average dry bulb temp (dbt) during the occupied hours of days
            v_Tdbt_Day = prod(m_mhdbt, clockHourOccupied);
            v_Tdbt_Day = div(v_Tdbt_Day, sum(clockHourOccupied));

            // monthly avg dbt during the unoccupied hours of days
            v_Tdbt_nt = prod(m_mhdbt, clockHourUnoccupied);
            v_Tdbt_nt = div(v_Tdbt_nt, sum(clockHourUnoccupied));

            // monthly avg global horiz rad power (Egh)  during the "day" hours
            Vector v_Egh_day = prod(m_mhEgh, clockHourOccupied);
            v_Egh_day = div(v_Egh_day, sum(clockHourOccupied));

            // monthly avg Egh during the "night" hours
            Vector v_Egh_nt = prod(m_mhEgh, clockHourUnoccupied);
            v_Egh_nt = div(v_Egh_nt, sum(clockHourUnoccupied));

            // Monthly avg Egh energy (Wgh) during the week days.
            Vector v_Wgh_wk_day = mult(v_Egh_day, weekdayOccupiedMegaseconds);
            // Monthly avg Wgh during week nights.
            Vector v_Wgh_wk_nt = mult(v_Egh_nt, weekdayUnoccupiedMegaseconds);
            // Monthly avg Wgh during weekend days.
            Vector v_Wgh_wke_day = mult(v_Egh_day, weekendOccupiedMegaseconds);
            // Monthly avg Wgh during weekend nights.
            Vector v_Wgh_wke_nt = mult(v_Egh_nt, weekendUnoccupiedMegaseconds);
            // Egh_avg_total MJ/m2.
            Vector v_Wgh_tot = sum(sum(v_Wgh_wk_day, v_Wgh_wk_nt), sum(v_Wgh_wke_day, v_Wgh_wke_nt));

            // frac_Egh_unocc_weekday_night
            frac_Pgh_wk_nt = div(v_Wgh_wk_nt, v_Wgh_tot);
            // frac_Egh_unocc_weekend_day
            frac_Pgh_wke_day = div(v_Wgh_wke_day, v_Wgh_tot);
            // frac_Egh_unocc_weekend_night
            frac_Pgh_wke_nt = div(v_Wgh_wke_nt, v_Wgh_tot);

            // Find what time the sun comes up and goes down and the fraction of hours sun is up and down.
            Vector v_frac_hrs_sun_down = Vector(12);
            Vector v_frac_hrs_sun_up = Vector(12);
            Vector v_sun_up_time = Vector(12);
            Vector v_sun_down_time = Vector(12);

            for (int i = 0; i < 12; i++) {
                v_frac_hrs_sun_up[i] = 0;
                v_frac_hrs_sun_down[i] = 0;

                // Searching fowards, the first hour with non-zero Egh is the first daylight hour (sunrise).
                for (int j = 0; j < 24; j++) {
                    if (m_mhEgh(i, j) != 0) {
                        v_sun_up_time[i] = j;
                        break;
                    }
                }

                // Searching backwards, the first hour with non-zero Egh is the last daylight hour (sunset is at the *end* of this hour).
                for (int j = 23; j >= 0; j--) {
                    if (m_mhEgh(i, j) != 0) {
                        v_sun_down_time[i] = j;
                        break;
                    }
                }

                // Fraction of hours the sun is up (add 1 to account for the fact that v_sun_down_time is the last daylight hour (i.e. that hour is still daytime).
                v_frac_hrs_sun_up[i] = (v_sun_down_time[i] - v_sun_up_time[i] + 1) / 24.0;
                // Fraction of hours the sun is down.
                v_frac_hrs_sun_down[i] = 1.0 - v_frac_hrs_sun_up[i];
            }

            // Nighttime hours per month.
            for (unsigned int i = 0; i < v_frac_hrs_sun_down.size(); i++) {
                v_hrs_sun_down_mo[i] = v_frac_hrs_sun_down[i] * hoursInMonth[i];
            }
        }

        /**
         * Compute lighting energy use as per prEN 15193:2006.
         */
        void MonthlyModel::lightingEnergyUse(const Vector& v_hrs_sun_down_mo, double& Q_illum_occ, double& Q_illum_unocc, double& Q_illum_tot_yr,
            Vector& v_Q_illum_tot, Vector& v_Q_illum_ext_tot) const
        {
            double lpd_occ = lights.powerDensityOccupied();
            double lpd_unocc = lights.powerDensityUnoccupied();

            // Daylight sensor dimming fraction.
            double F_D = lights.dimmingFraction();
            // Occupancy sensor control fraction.
            double F_O = building.lightingOccupancySensor();
            // Constant illimance control fraction.
            double F_C = building.constantIllumination();

            // TODO: The following assumes day starts at hour 7 and ends at hour 19
            // and 2 weeks per year are considered completely unoccupied for lighting
            // This should be converted to a monthly quanitity using the monthly
            // average sunup and sundown times.

            // Lighting operational hours during the daytime.
            double hoursOccupied = std::min(lights.n_day_end(), pop.hoursEnd()) - std::max(pop.hoursStart(), lights.n_day_start());
            if (hoursOccupied < 0) {
                hoursOccupied += 24;
            }
            double daysOccupied = pop.daysEnd() - pop.daysStart() + 1;
            if (daysOccupied < 0) {
                daysOccupied += 7;
            }
            double t_lt_D = hoursOccupied * daysOccupied * lights.n_weeks();

            // Lighting operational hours during the nighttime.
            hoursOccupied = std::max(lights.n_day_start() - pop.hoursStart(), 0.0) + std::max(pop.hoursEnd() - lights.n_day_end(), 0.0);
            double t_lt_N = hoursOccupied * daysOccupied * lights.n_weeks();

            // Unoccupied hours.
            double t_unocc = hoursInYear - t_lt_D - t_lt_N;

            // Total lighting energy for occupied times (kWh).
            Q_illum_occ = structure.floorArea() * lpd_occ * F_C * F_O * (t_lt_D * F_D + t_lt_N) / 1000.0;
            // Total annual lighting energy for unnocupied times (kWh).
            Q_illum_unocc = structure.floorArea() * lpd_unocc * t_unocc / 1000.0;
            // Total annual lighting energy (kWh).
            Q_illum_tot_yr = Q_illum_occ + Q_illum_unocc;

            // Split annual lighting energy into monthly lighting energy via the month fraction of the year (kWh).
            v_Q_illum_tot = mult(monthFractionOfYear, Q_illum_tot_yr, 12);
            // Total exterior lighting (kWh).
            v_Q_illum_ext_tot = mult(v_hrs_sun_down_mo, lights.exteriorEnergy() / 1000.0);
        }

        /**
         * Compute envelope parameters as per ISO 13790 8.3.
         */
        void MonthlyModel::envelopCalculations(Vector& v_win_A, Vector& v_wall_emiss, Vector& v_wall_alpha_sc, Vector& v_wall_U, Vector& v_wall_A,
            double& H_tr) const
        {
            // TODO: Copying the various structure values to new variables (e.g. v_wall_A) is not necessary. BAA@2015-07-13.
            v_wall_A = structure.wallArea();
            v_win_A = structure.windowArea();
            v_wall_U = structure.wallUniform();
            Vector v_win_U = structure.windowUniform();

            // Compute total envelope U*A.
            Vector v_env_UA = sum(mult(v_wall_A, v_wall_U), mult(v_win_A, v_win_U));

            // Compute direct transmission heat transfer coefficient to exterior in as per ISO 13790 8.3.1 (W/K).
            // Ignore linear and point thermal bridges for now.
            // TODO: Implement thermal bridges. BAA@2015-07-13.
            double H_D = sum(v_env_UA);

            // For now, also ignore heat transfer to ground (minimal in large buildings), unconditioned spaces, and adjacent buildings.
            // TODO: Implement ground, unconditioned, and adjacent above heat transfer coefficients. BAA@2015-07-13.
            double H_g = 0;
            double H_U = 0;
            double H_A = 0;

            // Total transmission heat transfer coefficient. ISO 13790 8.3.1 eq. 17.
            H_tr = H_D + H_g + H_U + H_A;

            v_wall_emiss = structure.wallThermalEmissivity();
            v_wall_alpha_sc = structure.wallSolarAbsorption();
        }

        /*
         * Compute window solar gain per ISO 13790 11.3.
         */
        void MonthlyModel::windowSolarGain(const Vector& v_win_A, const Vector& v_wall_emiss, const Vector& v_wall_alpha_sc, const Vector& v_wall_U,
            const Vector& v_wall_A, Vector& v_wall_A_sol, Vector& v_win_hr, Vector& v_wall_R_sc, Vector& v_win_A_sol) const
        {
            // TODO: The solar heat gain could be improved
            // better understand SCF and SDF and how they map to F_sh
            // calculate effective sky temp so we can better estimate theta_er and
            // theta_ss, and hr.

            // From ISO 13790 11.3.3 Effective solar collecting area of glazed elements, eqn 44
            // A_sol = F_sh,gl* g_gl*(1 ? F_f)*A_w,p
            // A_sol = effective solar collecting area of window in m2
            // F_sh,gl = shading reduction factor for movable shades as per 11.4.3 (v_win_SDF *v_win_SDF_frac)
            // g_gl = total solar energy transmittance of transparent element as per 11.4.2
            // F_f = Frame area fraction (ratio of projected frame area to overall glazed element area) as per 11.4.5 (v_wind_ff)
            // A_w,p = ovaral projected area of glazed element in m2 (v_wind_A)

            int vsize = 9; // 8 compass directions + roof = 9.

            // Frame factor.
            Vector v_win_ff = Vector(vsize);

            double n_win_SDF_table[] = { 0.5, 0.35, 1.0 };
            Vector v_win_SDF = Vector(vsize);
            Vector v_win_SDF_frac = Vector(vsize);

            for (int i = 0; i < vsize; i++) {
                v_win_ff[i] = 1.0 - structure.win_ff();
                // Assign SDF based on pulldown value of 1, 2 or 3.
                // TODO: This needs to be clarified in the .ism file as it's not obvious that the
                // window SDF is a magic number rather than the actual value. BAA@2015-07-13 BAA@2015-07-143
                v_win_SDF[i] = n_win_SDF_table[((int)structure.windowShadingDevice()[i]) - 1];
                // Set the SDF fractions which include heat transfer - set at 100% for now.
                v_win_SDF_frac[i] = 1.0;
            }

            Vector v_win_F_shgl = mult(v_win_SDF, v_win_SDF_frac);

            // Normal incidence solar energy transmittance which is SHGC in america.
            Vector v_g_gln = structure.windowNormalIncidenceSolarEnergyTransmittance();
            // Solar energy transmittance of glazing as per ISO 13790 11.4.2.
            Vector v_g_gl = mult(v_g_gln, structure.win_F_W());

            v_win_A_sol = mult(mult(mult(v_win_F_shgl, v_g_gl), v_win_ff), v_win_A);

            // Form factors given in ISO 13790, 11.4.6 as 0.5 for wall, 1.0 for unshaded roof
            double n_v_env_form_factors[] =
            { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1 };

            // Vertical wall external convective surface heat resistances.
            v_wall_R_sc = Vector(vsize);
            for (int i = 0; i < vsize; i++) {
                v_wall_R_sc[i] = structure.R_sc_ext();
            }

            // Window external radiative heat xfer coeff.
            // ISO 13790 11.4.6 says use hr=5 as a first approx.
            v_win_hr = mult(v_wall_emiss, 5.0);

            v_wall_A_sol = mult(mult(mult(v_wall_alpha_sc, v_wall_R_sc), v_wall_U), v_wall_A);
        }

        /**
         * Calculate solar heat gain. ISO 13790 11.3.2.
         */
        void MonthlyModel::solarHeatGain(const Vector& v_win_A_sol, const Vector& v_wall_R_sc, const Vector& v_wall_U, const Vector& v_wall_A,
            const Vector& v_win_hr, const Vector& v_wall_A_sol, Vector& v_E_sol) const
        {
            // EN ISO 13790 11.3.2 eq. 43.
            // \Phi_sol,k = F_sh,ob,k * A_sol,k * I_sol,k - F_r,k * \Phi_r,k

            // \Phi_sol,k = solar heat flow gains through building element k
            // F_sh,ob,k = shading reduction factor for external obstacles calculated via 11.4.4
            // A_sol,k = effective collecting area of surface calculated via 11.3.3 (glazing ) 11.3.4 (opaque)
            // I_sol,k = solar irradiance, mean energy of solar irradiation per square meter calculated using Annex F
            // F_r,k form factor between building and sky determined using 11.4.6
            // \Phi_r,k extra heat flow from thermal radiation to sky determined using 11.3.5

            // TODO: The solar heat gain could be improved
            // better understand SCF and SDF and how they map to F_SH
            // calculate effective sky temp so we can better estimate theta_er and
            // theta_ss.

            Vector v_win_SCF_frac(structure.windowShadingCorrectionFactor().size());
            for (unsigned int i = 0; i < v_win_SCF_frac.size(); i++) {
                // SCF fraction to include in HX. Fixed at 100% for now.
                v_win_SCF_frac[i] = 1;
            }

            // Combine vertical surface radiation (mosolar) and horizontal radiation (mEgh) into one matrix (W/m2).
            Matrix m_I_sol(12, 9); // 12 months, 8 directions + 1 roof.
            for (unsigned int r = 0; r < m_I_sol.size1(); r++) {
                for (unsigned int c = 0; c < m_I_sol.size2() - 1; c++) {
                    m_I_sol(r, c) = location.weather()->msolar()(r, c);
                }
                m_I_sol(r, m_I_sol.size2() - 1) = location.weather()->mEgh()[r];
            }
            printMatrix("m_I_sol", m_I_sol);

            // Compute the total solar heat gain for the glazing area.
            Vector v_win_phi_sol(12);
            Vector temp(9);
            for (unsigned int i = 0; i < v_win_phi_sol.size(); i++) {
                for (unsigned int j = 0; j < temp.size(); j++) {
                    temp[j] = structure.windowShadingCorrectionFactor()[j] * v_win_SCF_frac[j] * v_win_A_sol[j] * m_I_sol(i, j);
                }
                v_win_phi_sol[i] = sum(temp);
            }

            // Compute opaque area thermal radiation to the sky from EN ISO 13790 11.3.5
            // \Phi_r,k = R_se * U_c  * A_c * h_h * \delta\theta_er (46)
            // \Phi_r,k = thermal radiation to sky in W
            // R_se = external heat resistance as defined above m2K/W
            // U_c = U value of element as defined above W/m2K
            // A_c = area of element  defined above m2
            // \delta\theta_er = is the average difference between the external air temperature and the apparent sky temperature,
            // determined in accordance with 11.4.6, expressed in degrees centigrade.

            Vector theta_er(9);
            for (unsigned int i = 0; i < theta_er.size(); i++) {
                // Average difference between air temperature and sky temperature.
                // ISO 13790 11.4.6 says take \Theta_er=9k in sub polar zones, 13 K in tropical or 11 K in intermediate
                // TODO: Does the .epw file contain the sky temperature? If not, use the weather file's lat/lon to
                // determine which default value to use for theta_er. BAA@2015-07-13.
                theta_er[i] = 11.0;
            }
            double n_v_env_form_factors[] =
            { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1 };

            Vector v_wall_phi_r = mult(mult(mult(mult(v_wall_R_sc, v_wall_U), v_wall_A), v_win_hr), theta_er);

            // Total solar heat gain for opaque area.
            Vector v_wall_phi_sol(12);

            // Compute the total solar heat gain for the opaque area.
            for (unsigned int i = 0; i < v_win_phi_sol.size(); i++) {
                for (unsigned int j = 0; j < temp.size(); j++) {
                    temp[j] = v_wall_A_sol[j] * m_I_sol(i, j) - v_wall_phi_r[j] * n_v_env_form_factors[j];
                }
                v_wall_phi_sol[i] = sum(temp);
            }

            printVector("v_wall_phi_r", v_wall_phi_r);
            printVector("v_win_phi_sol", v_win_phi_sol);
            printVector("v_wall_phi_sol", v_wall_phi_sol);

            // Total envelope solar heat gain (W).
            Vector v_phi_sol = sum(v_win_phi_sol, v_wall_phi_sol);
            printVector("v_phi_sol", v_phi_sol);

            // Total envelope solar heat gain (MJ).
            v_E_sol = mult(v_phi_sol, megasecondsInMonth);
        }

        /**
         * Compute internal heat gains and losses.
         */
        void MonthlyModel::heatGainsAndLosses(double frac_hrs_wk_day, double Q_illum_occ, double Q_illum_unocc, double Q_illum_tot_yr, double& phi_int_avg,
            double& phi_plug_avg, double& phi_illum_avg, double& phi_int_wke_nt, double& phi_int_wke_day, double& phi_int_wk_nt) const
        {
            // Internal heat gains from people (W/m2).
            double phi_int_occ = pop.heatGainPerPerson() / pop.densityOccupied();
            double phi_int_unocc = pop.heatGainPerPerson() / pop.densityUnoccupied();
            phi_int_avg = frac_hrs_wk_day * phi_int_occ + (1 - frac_hrs_wk_day) * phi_int_unocc;

            // Internal heat gain from appliances (W/m2).
            double phi_plug_occ = building.electricApplianceHeatGainOccupied() + building.gasApplianceHeatGainOccupied();
            double phi_plug_unocc = building.electricApplianceHeatGainUnoccupied() + building.gasApplianceHeatGainUnoccupied();
            phi_plug_avg = phi_plug_occ * frac_hrs_wk_day + phi_plug_unocc * (1 - frac_hrs_wk_day);

            // Internal heat gain from illumination (W/m2).
            double phi_illum_occ = Q_illum_occ / structure.floorArea() / hoursInYear / frac_hrs_wk_day * 1000;
            double phi_illum_unocc = Q_illum_unocc / structure.floorArea() / hoursInYear / (1 - frac_hrs_wk_day) * 1000;
            phi_illum_avg = Q_illum_tot_yr / structure.floorArea() / hoursInYear * 1000;


            // Original spreadsheet computed the approximate internal heat gain for week nights, weekend days, and weekend nights
            // assuming they scale as the occ. fractions.  These are used for finding temp and not for directly calculating energy
            // use total so approximations are more acceptable.
            //
            // The following is a more accuate internal heat gain for week nights,
            // weekend days and weekend nights as it uses the unoccupied values rather
            // than just scaling occupied versions with the occupancy fraction
            // RTM 13-Nov-2012
            phi_int_wk_nt = (phi_int_unocc + phi_plug_unocc + phi_illum_unocc);
            phi_int_wke_day = (phi_int_unocc + phi_plug_unocc + phi_illum_unocc);
            phi_int_wke_nt = (phi_int_unocc + phi_plug_unocc + phi_illum_unocc);
        }

        /**
         * Compute total internal heat gain in W.
         */
        void MonthlyModel::internalHeatGain(double phi_int_avg, double phi_plug_avg, double phi_illum_avg, double& phi_I_tot) const
        {
            // Total occupant internal heat gain per year (W).
            double phi_I_occ = phi_int_avg * structure.floorArea();

            // Total appliance internal heat gain per year (W).
            double phi_I_app = phi_plug_avg * structure.floorArea();

            // Total lighting internal heat gain per year (W).
            double phi_I_lt = phi_illum_avg * structure.floorArea();

            // Total internal heat gain (W).
            phi_I_tot = phi_I_occ + phi_I_app + phi_I_lt;
        }

        /**
         * Compute unoccupied heat gain.
         */
        void MonthlyModel::unoccupiedHeatGain(double phi_int_wk_nt, double phi_int_wke_day, double phi_int_wke_nt, const Vector& weekdayUnoccupiedMegaseconds,
            const Vector& weekendOccupiedMegaseconds, const Vector& weekendUnoccupiedMegaseconds, const Vector& frac_Pgh_wk_nt,
            const Vector& frac_Pgh_wke_day, const Vector& frac_Pgh_wke_nt, const Vector& v_E_sol, Vector& v_P_tot_wke_day, Vector& v_P_tot_wk_nt,
            Vector& v_P_tot_wke_nt) const
        {
            // Internal heat gain for unoccupied times (MJ).
            Vector v_W_int_wk_nt = mult(weekdayUnoccupiedMegaseconds, phi_int_wk_nt * structure.floorArea());
            Vector v_W_int_wke_day = mult(weekendOccupiedMegaseconds, phi_int_wke_day * structure.floorArea());
            Vector v_W_int_wke_nt = mult(weekendUnoccupiedMegaseconds, phi_int_wke_nt * structure.floorArea());
            printVector("v_W_int_wk_nt", v_W_int_wk_nt);
            printVector("v_W_int_wke_day", v_W_int_wke_day);
            printVector("v_W_int_wke_nt", v_W_int_wke_nt);

            // Solar heat gain for unoccupied times (MJ).
            Vector v_W_sol_wk_nt = mult(v_E_sol, frac_Pgh_wk_nt);
            Vector v_W_sol_wke_day = mult(v_E_sol, frac_Pgh_wke_day);
            Vector v_W_sol_wke_nt = mult(v_E_sol, frac_Pgh_wke_nt);
            printVector("v_W_sol_wk_nt", v_W_sol_wk_nt);
            printVector("v_W_sol_wke_day", v_W_sol_wke_day);
            printVector("v_W_sol_wke_nt", v_W_sol_wke_nt);

            // Total heat gain for unoccupied times (MJ).
            v_P_tot_wk_nt = div(sum(v_W_int_wk_nt, v_W_sol_wk_nt), weekdayUnoccupiedMegaseconds);
            v_P_tot_wke_day = div(sum(v_W_int_wke_day, v_W_sol_wke_day), weekendOccupiedMegaseconds);
            v_P_tot_wke_nt = div(sum(v_W_int_wke_nt, v_W_sol_wke_nt), weekendUnoccupiedMegaseconds);
        }

        /*
         * Calculate interior temp.
         */
        void MonthlyModel::interiorTemp(const Vector& v_wall_A, const Vector& v_P_tot_wke_day, const Vector& v_P_tot_wk_nt, const Vector& v_P_tot_wke_nt,
            const Vector& v_Tdbt_nt, const Vector& v_Tdbt_day, double H_tr, double hoursUnoccupiedPerDay, double hoursOccupiedPerDay, double frac_hrs_wk_day, double frac_hrs_wk_nt,
            double frac_hrs_wke_tot, Vector& v_Th_avg, Vector& v_Tc_avg, double& tau) const
        {
            // Set the temp differential from the interior heating/cooling setpoint
            // based on the BEM type. An advanced BEM has the effect of reducing the
            // effective heating temp and raising the effective cooling temp during
            // times of control (i.e. during occupancy).
            double T_adj = 0;
            switch ((int)building.buildingEnergyManagement()) {
            case 1:
                T_adj = 0.0;
                break;
            case 2:
                T_adj = 0.5;
                break;
            case 3:
                T_adj = 1.0;
                break;
            }

            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "BEM: " << building.buildingEnergyManagement() << ", " << ((int)building.buildingEnergyManagement()) << std::endl;
                std::cout << "T_adj: " << T_adj << std::endl;
            }

            // Adjust the heating set points.
            double ht_tset_ctrl = heating.temperatureSetPointOccupied() - T_adj;
            double cl_tset_ctrl = cooling.temperatureSetPointOccupied() + T_adj;

            // During unoccupied times, we use a setback temp and even if we have a BEM
            // it has no effect.
            double ht_tset_unocc = heating.temperatureSetPointUnoccupied();
            double cl_tset_unocc = cooling.temperatureSetPointUnoccupied();

            Vector v_ht_tset_ctrl(12);
            Vector v_cl_tset_ctrl(12);

            // Create vectors of the adjusted heating set points.
            for (unsigned int i = 0; i < v_cl_tset_ctrl.size(); i++) {
                v_cl_tset_ctrl[i] = cl_tset_ctrl;
                v_ht_tset_ctrl[i] = ht_tset_ctrl;
            }

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_cl_tset_ctrl", v_cl_tset_ctrl);
                printVector("v_ht_tset_ctrl", v_ht_tset_ctrl);
            }

            // Interior heat capacity (J/k).
            double Cm_int = structure.interiorHeatCapacity() * structure.floorArea();

            // Envelope heat capacity (J/k).
            double Cm_env = structure.wallHeatCapacity() * sum(v_wall_A);

            // Total heat capacity (J/k).
            double Cm = Cm_int + Cm_env;

            // Total heat transfer coefficient.
            double H_tot = H_tr + ventilation.H_ve();

            // Building time constant in hours as pwer ISO 13790 12.2.1.3 eq. 62.
            tau = Cm / H_tot / 3600.0;

            // The following code computes the average weekend room temp using exponential rise and
            // decays as we switch between day and night temp settings.  It assumes that
            // the weekend is two days (we'll call them sat and sun)
            //
            // we do this wierd breakdown breakdown because want to separate day with
            // solar loading from night without.  We can then use the average temp
            // in each time frame rather than the overall monthly average.  right now
            // wk_nt stuff is the same as wke_nt, but wke_day is much different because
            // the solar gain increases the heat gain considerably, even on the weekend
            // when occupant, lighting, and plugload gains are small

            // Create a vector of lengths of the periods of times between possible temperature resets during
            // the weekend.
            Vector v_ti(5);
            v_ti[0] = v_ti[2] = v_ti[4] = hoursUnoccupiedPerDay;
            v_ti[1] = v_ti[3] = hoursOccupiedPerDay;

            // Generate an effective delta T matrix from ratio of total interior gains to heat
            // transfer coefficient for each time period.
            //
            // This is a matrix where the columns are the vectors v_P_tot_wk_nt/H_tot, and so on
            // this is for a week night, weekend day, weekend night, weekend day, weekend night sequence
            Matrix M_dT(v_P_tot_wk_nt.size(), 5);
            Matrix M_Te(v_Tdbt_nt.size(), 5);

            for (unsigned int i = 0; i < v_P_tot_wk_nt.size(); ++i) {
                M_dT(i, 0) = v_P_tot_wk_nt[i] / H_tot;
                M_dT(i, 1) = M_dT(i, 3) = v_P_tot_wke_day[i] / H_tot;
                M_dT(i, 2) = M_dT(i, 4) = v_P_tot_wke_nt[i] / H_tot;
            }

            for (unsigned int i = 0; i < v_Tdbt_nt.size(); ++i) {
                M_Te(i, 0) = M_Te(i, 2) = M_Te(i, 4) = v_Tdbt_nt[i];
                M_Te(i, 1) = M_Te(i, 3) = v_Tdbt_day[i];
            }

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printMatrix("M_dT", M_dT);
                printMatrix("M_Te", M_Te);
                printVector("v_ti", v_ti);
            }

            Vector v_Th_wke_avg(v_ht_tset_ctrl);
            Vector v_Th_wk_day(v_ht_tset_ctrl);
            Vector v_Th_wk_nt(v_ht_tset_ctrl);

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Th_wke_avg", v_Th_wke_avg);
                printVector("v_Th_wk_day", v_Th_wk_day);
                printVector("v_Th_wk_nt", v_Th_wk_nt);
            }

            // Compute the change in temp from setback to another heating temp in unoccupied times
            if (heating.T_ht_ctrl_flag() == 1) { // If the HVAC heating controls are turned on.
                Matrix M_Ta(12, 4);
                Vector v_Tstart(v_ht_tset_ctrl);
                for (unsigned int i = 0; i < M_Ta.size2(); i++) {
                    for (unsigned int j = 0; j < M_Ta.size1(); j++) {
                        v_Tstart[j] = M_Ta(j, i) = (v_Tstart[j] - M_Te(j, i) - M_dT(j, i)) * exp(-1 * v_ti[i] / tau) + M_Te(j, i) + M_dT(j, i);
                    }
                }

                if (DEBUG_ISO_MODEL_SIMULATION) {
                    printMatrix("M_Ta", M_Ta);
                    printVector("v_Tstart", v_Tstart);
                }

                if (DEBUG_ISO_MODEL_SIMULATION) {
                    printVector("v_cl_tset_ctrl", v_cl_tset_ctrl);
                    printVector("v_ht_tset_ctrl", v_ht_tset_ctrl);
                }

                // Find the exponential Temp decay after any changes in heating temp setpoint and put
                // in the matrix M_Ta with columns being the different time segments.
                // The temp will only decay to the new lower setpoint, so find which is
                // higher the setpoint or the decay and select that as the start point for
                // the average integration to follow.
                Matrix M_Taa(12, 5);
                for (unsigned int j = 0; j < M_Taa.size1(); j++) {
                    M_Taa(j, 0) = v_ht_tset_ctrl[j];
                }

                if (DEBUG_ISO_MODEL_SIMULATION) {
                    printMatrix("M_Taa", M_Taa);
                }

                for (unsigned int i = 1; i < M_Taa.size2(); i++) {
                    for (unsigned int j = 0; j < M_Taa.size1(); j++) {
                        M_Taa(j, i) = std::max(M_Ta(j, i - 1), ht_tset_unocc);
                    }
                }

                if (DEBUG_ISO_MODEL_SIMULATION) {
                    printMatrix("M_Taa", M_Taa);
                }

                Matrix M_Tb(12, 5);

                // For each time period, find the average temp given the start and
                // ending temp and assuming exponential decay of temps.
                // Loop through wk nt to wke day to wke nt to wke day to wke nt.
                for (unsigned int i = 0; i < M_Tb.size2(); i++) {
                    for (unsigned int j = 0; j < M_Tb.size1(); j++) {
                        double v_T_avg = tau / v_ti[i] * (M_Taa(j, i) - M_Te(j, i) - M_dT(j, i)) * (1 - exp(-1 * v_ti[i] / tau)) + M_Te(j, i) + M_dT(j, i);
                        M_Tb(j, i) = std::max(v_T_avg, ht_tset_unocc);
                    }
                }
                for (unsigned int i = 0; i < v_Th_wke_avg.size(); i++) {
                    double rowSum = 0;
                    for (unsigned int j = 0; j < M_Tb.size2(); j++) {
                        rowSum += M_Tb(i, j);
                    }
                    v_Th_wke_avg[i] = rowSum / M_Tb.size2();
                }
                for (unsigned int j = 0; j < M_Tb.size1(); j++) {
                    v_Th_wk_nt[j] = M_Tb(j, 1);
                }

                if (DEBUG_ISO_MODEL_SIMULATION) {
                    printMatrix("M_Tb", M_Tb);
                    printVector("v_Th_wke_avg", v_Th_wke_avg);
                    printVector("v_Th_wk_nt", v_Th_wk_nt);
                }
            }

            // Default for if cooling is turned off.
            Vector v_Tc_wk_day(v_cl_tset_ctrl);
            Vector v_Tc_wk_nt(v_cl_tset_ctrl);
            Vector v_Tc_wke_avg(v_cl_tset_ctrl);

            // If cooling is on, find the temp decay after any changes in cooling temp setpoint.
            // TODO: Consider pulling this giant if statement into its own function. -BAA@2015-07-14
            if (cooling.T_cl_ctrl_flag() == 1) {
                Matrix M_Tc(12, 4);
                Vector v_Tstart(v_cl_tset_ctrl);
                for (unsigned int i = 0; i < M_Tc.size2(); i++) {
                    for (unsigned int j = 0; j < M_Tc.size1(); j++) {
                        v_Tstart[j] = M_Tc(j, i) = (v_Tstart[j] - M_Te(j, i) - M_dT(j, i)) * exp(-1 * v_ti[i] / tau) + M_Te(j, i) + M_dT(j, i);
                    }
                }

                // Check to see if the decay temp is lower than the temp setpoint.  If so, the space will cool
                // to that level. If the cooling setpoint is lower the cooling system will kick in and lower the
                // temp to the cold temp setpoint.
                Matrix M_Tcc(12, 5);
                for (unsigned int j = 0; j < M_Tcc.size1(); j++) {
                    M_Tcc(j, 0) = std::min(v_ht_tset_ctrl[j], cl_tset_unocc);
                }
                for (unsigned int i = 1; i < M_Tcc.size2(); i++) {
                    for (unsigned int j = 0; j < M_Tcc.size1(); j++) {
                        M_Tcc(j, i) = std::max(M_Tc(j, i - 1), cl_tset_unocc);
                    }
                }

                if (DEBUG_ISO_MODEL_SIMULATION) {
                    printMatrix("M_Tcc", M_Tcc);
                }

                // For each time period, find the average temp given the exponential decay.
                Matrix M_Td(12, 5);

                for (unsigned int i = 0; i < M_Td.size2(); i++) {
                    for (unsigned int j = 0; j < M_Td.size1(); j++) {
                        double v_T_avg = tau / v_ti[i] * (M_Tcc(j, i) - M_Te(j, i) - M_dT(j, i)) * (1 - exp(-1 * v_ti[i] / tau)) + M_Te(j, i) + M_dT(j, i);
                        if (DEBUG_ISO_MODEL_SIMULATION) {
                            std::cout << "v_T_avg = " << v_T_avg << std::endl;
                        }
                        M_Td(j, i) = std::max(v_T_avg, cl_tset_unocc);
                    }
                }


                if (DEBUG_ISO_MODEL_SIMULATION) {
                    printMatrix("M_Td", M_Td);
                }

                for (unsigned int i = 0; i < v_Th_wke_avg.size(); i++) {
                    double rowSum = 0;
                    for (unsigned int j = 0; j < M_Td.size2(); j++) {
                        rowSum += M_Td(i, j);
                    }
                    v_Tc_wke_avg[i] = rowSum / M_Td.size2();
                }
                for (unsigned int j = 0; j < M_Td.size1(); j++) {
                    v_Tc_wk_nt[j] = M_Td(j, 1);
                }
            }

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Tc_wk_day", v_Tc_wk_day);
                printVector("v_Tc_wk_nt", v_Tc_wk_nt);
                printVector("v_Tc_wke_avg", v_Tc_wke_avg);
            }

            // Find the average temp for the whole week from the fractions of each period.
            Vector v_Th_wk_avg = sum(sum(mult(v_Th_wk_day, frac_hrs_wk_day), mult(v_Th_wk_nt, frac_hrs_wk_nt)), mult(v_Th_wke_avg, frac_hrs_wke_tot));
            Vector v_Tc_wk_avg = sum(sum(mult(v_Tc_wk_day, frac_hrs_wk_day), mult(v_Tc_wk_nt, frac_hrs_wk_nt)), mult(v_Tc_wke_avg, frac_hrs_wke_tot));

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Tc_wk_avg", v_Tc_wk_avg);
                printVector("v_Th_wk_avg", v_Th_wk_avg);
            }

            // The final avg for monthly energy computations is the lesser of the avg
            // computed above and the heating set control.
            for (unsigned int i = 0; i < v_Tc_wk_avg.size(); i++) {
                v_Th_avg[i] = std::min(v_Th_wk_avg[i], ht_tset_ctrl);
                v_Tc_avg[i] = std::min(v_Tc_wk_avg[i], cl_tset_ctrl);
            }
        }

        /**
         * Calculate required energy for mechanical ventilation based on source EN ISO 13789
         * C.3, C.5 and EN 15242:2007 6.7 and EN ISO 13790 Sec 9.2.
         */
        void MonthlyModel::ventilationCalc(const Vector& v_Th_avg, const Vector& v_Tc_avg, double frac_hrs_wk_day, Vector& v_Hve_ht, Vector& v_Hve_cl) const
        {
            // Ventilation Zone Height (m) with a minimum of 0.1 m.
            double vent_zone_height = std::max(0.1, structure.buildingHeight());

            // Vent supply rate m3/h/m2 (input is in in L/s).
            double qv_supp = ventilation.supplyRate() / structure.floorArea() / 3.6;

            // Vent exhaust rate m3/h/m2, negative indicates out of building.
            double qv_ext = -(qv_supp - ventilation.supplyDifference() / structure.floorArea() / 3.6);

            // Combustion appliance ventilation rate - not implemented yet but will be impt for restaurants.
            double qv_comb = 0;

            // Difference between air intake and air exhaust including combustion exhaust.
            double qv_diff = qv_supp + qv_ext + qv_comb;

            double vent_ht_recov = ventilation.heatRecoveryEfficiency();

            double vent_outdoor_frac = 1 - ventilation.exhaustAirRecirculated();

            // Infilatration source EN 15242:2007 Sec 6.7 direct method
            double tot_env_A = sum(structure.wallArea()) + sum(structure.windowArea());

            // Infiltration data from:
            // Tamura, (1976), Studies on exterior wall air tightness and air infiltration of tall buildings, ASHRAE Transactions, 82(1), 122-134.
            // Orm (1998), AIVC TN44: Numerical data for air infiltration and natural ventilation calculations, Air Infiltration and Ventilation Centre.
            // Emmerich, (2005), Investigation of the Impact of Commercial Building Envelope Airtightness on HVAC Energy Use.

            // Infiltration rate in m3/h/m2 @ 75 Pa based on wall area.
            double v_Q75pa = structure.infiltrationRate();

            // Convert infiltration to Q@4Pa in m3/h /m2 based on floor area.
            // double v_Q4pa = v_Q75pa * tot_env_A / structure.floorArea() * (std::pow((4.0 / 75.0), ventilation.p_exp()));
            double v_Q4pa = v_Q75pa;

            // Effective stack height.
            double h_stack = ventilation.zone_frac() * vent_zone_height;

            Vector dbtDiff = dif(location.weather()->mdbt(), v_Th_avg);
            printVector("dbtDiff", dbtDiff);
            Vector dbtDiffAbs = abs(dbtDiff);
            printVector("dbtDiffAbs", dbtDiffAbs);
            Vector dbtHStack = mult(dbtDiffAbs, h_stack);
            printVector("dbtHstack", dbtHStack);
            Vector dbtPowered = pow(dbtHStack, ventilation.stack_exp());
            printVector("dbtPowered", dbtPowered);
            Vector dbtMultQ4 = mult(dbtPowered, ventilation.stack_coeff() * v_Q4pa);
            printVector("dbtMultQ4", dbtMultQ4);

            // Calculate the infiltration from stack effect pressure difference for heating from EN 15242: sec 6.7.1 (m3/h/m2).
            Vector v_qv_stack_ht = maximum(dbtMultQ4, 0.001);

            // Recalculate for cooling.
            dbtDiff = dif(location.weather()->mdbt(), v_Tc_avg);
            printVector("dbtDiff", dbtDiff);
            dbtDiffAbs = abs(dbtDiff);
            printVector("dbtDiffAbs", dbtDiffAbs);
            dbtHStack = mult(dbtDiffAbs, h_stack);
            printVector("dbtHstack", dbtHStack);
            dbtPowered = pow(dbtHStack, ventilation.stack_exp());
            printVector("dbtPowered", dbtPowered);
            dbtMultQ4 = mult(dbtPowered, ventilation.stack_coeff() * v_Q4pa);
            printVector("dbtMultQ4", dbtMultQ4);

            // Calculate the infiltration from stack effect pressure difference for cooling from EN 15242: sec 6.7.1 (m3/h/m2).
            Vector v_qv_stack_cl = maximum(dbtMultQ4, 0.001);
            printVector("v_qv_stack_ht", v_qv_stack_ht);
            printVector("v_qv_stack_cl", v_qv_stack_cl);

            Vector v_qv_wind_ht = mult(mult(pow(mult(mult(location.weather()->mwind(), location.weather()->mwind()), ventilation.dCp() * location.terrain()),
                ventilation.wind_exp()), v_Q4pa), ventilation.wind_coeff());
            Vector v_qv_wind_cl = mult(mult(pow(mult(mult(location.weather()->mwind(), location.weather()->mwind()), ventilation.dCp() * location.terrain()),
                ventilation.wind_exp()), v_Q4pa), ventilation.wind_coeff());
            printVector("v_qv_wind_ht", v_qv_wind_ht);
            printVector("v_qv_wind_cl", v_qv_wind_cl);

            Vector v_qv_ht_max = maximum(v_qv_stack_ht, v_qv_wind_ht);
            Vector v_qv_cl_max = maximum(v_qv_stack_cl, v_qv_wind_cl);
            printVector("v_qv_ht_max", v_qv_ht_max);
            printVector("v_qv_cl_max", v_qv_cl_max);

            double n_sw_coeff = 0.14;
            Vector v_qv_sw_ht = sum(v_qv_ht_max, div(mult(mult(v_qv_stack_ht, v_qv_wind_ht), n_sw_coeff), v_Q4pa)); // m3/h/m2
            Vector v_qv_sw_cl = sum(v_qv_cl_max, div(mult(mult(v_qv_stack_cl, v_qv_wind_cl), n_sw_coeff), v_Q4pa)); // m3/h/m2
            printVector("v_qv_sw_ht", v_qv_sw_ht);
            printVector("v_qv_sw_cl", v_qv_sw_cl);

            Vector v_qv_inf_ht = sum(v_qv_sw_ht, std::max(0.0, -qv_diff)); // m3/h/m2
            Vector v_qv_inf_cl = sum(v_qv_sw_cl, std::max(0.0, -qv_diff)); // m3/h/m2
            printVector("v_qv_inf_ht", v_qv_inf_ht);
            printVector("v_qv_inf_cl", v_qv_inf_cl);

            // TODO: Figure out what the comment below is refering to. I don't want to delete it just yet
            // because connecting the code to the sources of the equations is important. BAA@2015-07-14.
            //
            // source EN ISO 13789 C.5  There they use Vdot instead of Q
            // Vdot = Vdot_f (1??_v) +Vdot_x
            // Vdot_f is the design airflow rate due to mechanical ventilation;
            // Vdot_x is the additional airflow rate with fans on, due to wind effects;
            // ?_v is the global heat recovery efficiency, taking account of the differences between supply and extract
            // airflow rates. Heat in air leaving the building through leakage cannot be recovered.

            // Set vent_rate_flag=0 if ventilation rate is constant, 1 if we assume vent off in unoccopied times or
            // 2 if we assume ventilation rate is dropped proportionally to population
            // set to 1 to mimic the behavior of the original spreadsheet.
            double vent_op_frac;
            switch (ventilation.vent_rate_flag()) {
            case 0:
                vent_op_frac = 1;
                break;
            case 1:
                vent_op_frac = frac_hrs_wk_day;
                break;
            default:
                vent_op_frac = frac_hrs_wk_day + (1 - frac_hrs_wk_day) * pop.densityOccupied() / pop.densityUnoccupied();
                break;
            }

            double initVal = ventilation.ventType() == 3 ? 0 : (vent_op_frac * qv_supp * vent_outdoor_frac * (1 - vent_ht_recov));
            Vector v_qv_mve_ht(12, initVal);
            Vector v_qv_mve_cl(12, initVal);

            // Total air flow in m3/s when heating.
            Vector v_qve_ht = sum(v_qv_inf_ht, v_qv_mve_ht);
            // Total air flow in m3/s when cooling.
            Vector v_qve_cl = sum(v_qv_inf_cl, v_qv_mve_cl);
            printVector("v_qve_ht", v_qve_ht);
            printVector("v_qve_cl", v_qve_cl);

            // Hve heating (W/K/m2).
            v_Hve_ht = div(mult(v_qve_ht, phys.rhoCpAir() * 1000000), 3600.0); // Multiply rhoCpAir by 1000000 to convert from MJ to W.
            // Hve cooling (W/K/m2).
            v_Hve_cl = div(mult(v_qve_cl, phys.rhoCpAir() * 1000000), 3600.0); // Multiply rhoCpAir by 1000000 to convert from MJ to W.
        }

        /**
         * Compute monthly heating and cooling demand.
         */
        void MonthlyModel::heatingAndCooling(const Vector& v_E_sol, const Vector& v_Th_avg, const Vector& v_Hve_ht, const Vector& v_Tc_avg,
            const Vector& v_Hve_cl, double tau, double H_tr, double phi_I_tot, double frac_hrs_wk_day, Vector& v_Qfan_tot, Vector& v_Qneed_ht,
            Vector& v_Qneed_cl, double& Qneed_ht_yr, double& Qneed_cl_yr) const
        {
            // Convert internal heat gains from W to MJ.
            Vector temp = mult(megasecondsInMonth, phi_I_tot, 12);

            // Total internal + solar heat gains (MJ).
            Vector v_tot_mo_ht_gain = sum(temp, v_E_sol);

            // Building heating dimensionless constant.
            double a_H = heating.a_H0() + tau / heating.tau_H0();

            // Heat transfer (loss) by transmission, heating (MJ).
            Vector v_QT_ht = mult(mult(dif(v_Th_avg, location.weather()->mdbt()), megasecondsInMonth), H_tr);
            // Heat transfer (loss) by ventilation, heating (MJ).
            Vector v_QV_ht = mult(mult(mult(v_Hve_ht, structure.floorArea()), dif(v_Th_avg, location.weather()->mdbt())), megasecondsInMonth);
            // Total heat transfer (loss) (MJ). ISO 13790 7.2.1.3 eq. 7.
            Vector v_Qtot_ht = sum(v_QT_ht, v_QV_ht);

            // Compute the ratio of heat gain to heat loss.
            Vector v_gamma_H_ht = div(v_tot_mo_ht_gain, sum(v_Qtot_ht, DBL_MIN)); // Add DBL_MIN to avoid divide by zero.

            // Heating utilization factor.
            Vector v_eta_g_H(12);

            // For each month, set the check the heat gain ratio and set the heating utlization factor accordingly.
            for (unsigned int i = 0; i < v_eta_g_H.size(); i++) {
                v_eta_g_H[i] =
                    v_gamma_H_ht[i] > 0 ? (1 - std::pow(v_gamma_H_ht[i], a_H)) / (1 - std::pow(v_gamma_H_ht[i], (a_H + 1))) : 1 / (v_gamma_H_ht[i] + DBL_MIN);
            }

            // Total heating need (MJ).
            v_Qneed_ht = dif(v_Qtot_ht, mult(v_eta_g_H, v_tot_mo_ht_gain));
            Qneed_ht_yr = sum(v_Qneed_ht);

            // Heat transfer (loss) by transmission, cooling (MJ).
            Vector v_QT_cl = mult(mult(dif(v_Tc_avg, location.weather()->mdbt()), H_tr), megasecondsInMonth);
            // Heat transfer (loss) by ventilation, cooling (MJ).
            Vector v_QV_cl = mult(mult(mult(v_Hve_cl, structure.floorArea()), dif(v_Tc_avg, location.weather()->mdbt())), megasecondsInMonth);
            // Total heat transfer (loss), cooling (MJ). ISO 13790 7.2.1.3 eq. 7.
            Vector v_Qtot_cl = sum(v_QT_cl, v_QV_cl);

            // Heat transfer (loss) to heat gain ratio, cooling.
            Vector v_gamma_H_cl = div(v_Qtot_cl, sum(v_tot_mo_ht_gain, DBL_MIN));

            // Compute the cooling gain utilization factor eta_g_cl
            Vector v_eta_g_CL(12);
            for (unsigned int i = 0; i < v_eta_g_CL.size(); i++) {
                if (DEBUG_ISO_MODEL_SIMULATION) {
                    double numer = (1.0 - std::pow(v_gamma_H_cl[i], a_H));
                    double denom = (1.0 - std::pow(v_gamma_H_cl[i], (a_H + 1.0)));
                    std::cout << numer << " = 1.0 - " << v_gamma_H_cl[i] << "^" << a_H << std::endl;
                    std::cout << denom << " = 1.0 - " << v_gamma_H_cl[i] << "^" << (a_H + 1.0) << std::endl;
                }
                v_eta_g_CL[i] = v_gamma_H_cl[i] > 0.0 ? (1.0 - std::pow(v_gamma_H_cl[i], a_H)) / (1.0 - std::pow(v_gamma_H_cl[i], (a_H + 1.0))) : 1.0;
            }

            // Total cooling need (MJ).
            v_Qneed_cl = dif(v_tot_mo_ht_gain, mult(v_eta_g_CL, v_Qtot_cl));
            Qneed_cl_yr = sum(v_Qneed_cl);

            // Hot air supply temperature (C).
            double T_sup_ht = heating.temperatureSetPointOccupied() + heating.dT_supp_ht();
            // Cool air supply temperature (C).
            double T_sup_cl = cooling.temperatureSetPointOccupied() - cooling.dT_supp_cl();

            // Volume of air moved for heating (m3).
            Vector v_Vair_ht = div(v_Qneed_ht, sum(mult(dif(T_sup_ht, v_Th_avg), phys.rhoCpAir()), DBL_MIN));
            // Volume of air moved for cooling (m3).
            Vector v_Vair_cl = div(v_Qneed_cl, sum(mult(dif(v_Tc_avg, T_sup_cl), phys.rhoCpAir()), DBL_MIN));

            printVector("v_Vair_ht", v_Vair_ht);
            printVector("v_Vair_cl", v_Vair_cl);

            // Total air flow (m3).
            // Multiply by 1000000 to convert megaseconds to seconds.
            // Divide by 1000 to convert liters to m3.
            Vector v_Vair_tot = maximum(sum(v_Vair_ht, v_Vair_cl), div(mult(megasecondsInMonth, ventilation.supplyRate() * frac_hrs_wk_day * 1000000.0, 12), 1000));
            printVector("v_Vair_tot", v_Vair_tot);

            // Fan power (MJ)
            // ventilation.fanPower is in W/L/s is also J/L which is also kJ/m3. Divide by 1000 for MJ/m3 to get fanEnergy in MJ.
            Vector fanEnergy = mult(v_Vair_tot, ventilation.fanPower() * ventilation.fanControlFactor() / 1000.0);
            printVector("fanEnergy", fanEnergy);

            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "ventilation.fanPower() = " << ventilation.fanPower() << std::endl;
                std::cout << "ventilation.fanControlFactor() = " << ventilation.fanControlFactor() << std::endl;
                std::cout << "structure.floorArea() = " << structure.floorArea() << std::endl;
            }

            // Calculate fan EUI (kWh/m2).
            v_Qfan_tot = div(div(fanEnergy, structure.floorArea()), 3.6);
        }

        /**
         * HVAC systems calculations.
         */
        void MonthlyModel::hvac(const Vector& v_Qneed_ht, const Vector& v_Qneed_cl, double Qneed_ht_yr, double Qneed_cl_yr, Vector& v_Qelec_ht, Vector& v_Qgas_ht,
            Vector& v_Qcl_elec_tot, Vector& v_Qcl_gas_tot) const
        {
            // TODO: Implement (or remove) all the district heating/cooling stuff that is currently commented out. BAA@2015-07-15.

            // From original matlab code. Preserved for future implementation of district heating/cooling. BAA@2015-07-15.
            /*
             %% District H/C info

             DH_YesNo =0;  % building connected to DH (0=no, 1=yes.  Assume DH is powered by natural gas)
             n_eta_DH_network = 0.9; % efficiency of DH network.  Typical value 0l75-0l9 EN 15316-4-5
             n_eta_DH_sys = 0.87; % efficiency of DH heating system
             n_frac_DH_free = 0.000; % fraction of free heat source to DH (0 to 1)

             DC_YesNo = 0;  % building connected to DC (0=no, 1=yes)
             n_eta_DC_network = 0.9;  % efficiency of DC network.
             n_eta_DC_COP = 5.5;  % COP of DC elec Chillers
             n_eta_DC_frac_abs = 0;  % fraction of DC chillers that are absorption
             n_eta_DC_COP_abs = 1;  % COP of DC absorption chillers
             n_frac_DC_free = 0;  % fraction of free heat source to absorption DC chillers (0 to 1)
             */

             // From EN 15243-2007 Annex E.
             // HVAC system info table from EN 15243:2007 Table E1.
             // The integrated energy efficiency ratio (IEER) is the effective average COP for the system.
            double IEER = cooling.cop() * cooling.partialLoadValue();

            // Copy over the HVAC loss/waste factors into local variables with names
            // that match the equations better
            double f_waste = heating.hotcoldWasteFactor();
            double a_ht_loss = heating.hvacLossFactor();
            double a_cl_loss = cooling.hvacLossFactor();

            // Fraction of yearly heating demand with regard to total heating + cooling demand.
            double f_dem_ht = std::max(Qneed_ht_yr / (Qneed_cl_yr + Qneed_ht_yr), 0.1);
            // Fraction of yearly cooling demand.
            double f_dem_cl = std::max((1.0 - f_dem_ht), 0.1);

            // Overall distribution efficiency for heating.
            double eta_dist_ht = 1.0 / (1.0 + a_ht_loss + f_waste / f_dem_ht);
            // Overall distrubtion efficiency for cooling.
            double eta_dist_cl = 1.0 / (1.0 + a_cl_loss + f_waste / f_dem_cl);

            // Losses from HVAC distributuion, heating.
            Vector v_Qloss_ht_dist = div(mult(v_Qneed_ht, (1 - eta_dist_ht)), eta_dist_ht);
            // Losses from HVAC distributuion, cooling.
            Vector v_Qloss_cl_dist = div(mult(v_Qneed_cl, (1 - eta_dist_cl)), eta_dist_cl);
            printVector("v_Qloss_ht_dist", v_Qloss_ht_dist);
            printVector("v_Qloss_cl_dist", v_Qloss_cl_dist);

            Vector v_Qht_sys(12, 0.0);
            Vector v_Qht_DH(12, 0.0);
            Vector v_Qcl_sys(12, 0.0);
            Vector v_Qcool_DC(12, 0.0);

            if (heating.DH_YesNo() == 1) {
                v_Qht_DH = sum(v_Qneed_ht, v_Qloss_ht_dist);
            }
            else {
                v_Qht_sys = div(sum(v_Qloss_ht_dist, v_Qneed_ht), heating.efficiency() + DBL_MIN);
            }

            if (cooling.DC_YesNo() == 1) {
                v_Qcool_DC = sum(v_Qneed_cl, v_Qloss_cl_dist);
            }
            else {
                v_Qcl_sys = div(sum(v_Qloss_cl_dist, v_Qneed_cl), IEER + DBL_MIN);
            }
            printVector("v_Qht_sys", v_Qht_sys);
            printVector("v_Qht_DH", v_Qht_DH);
            printVector("v_Qcl_sys", v_Qcl_sys);
            printVector("v_Qcool_DC", v_Qcool_DC);

            // From original matlab code. Preserved for future implementation of district heating/cooling. BAA@2015-07-15.
            /*
             if DH_YesNo==1
             v_Qht_sys = zeros(12,1);  % if we have district heating our heating energy needs from our system are zero
             v_Qht_DH = v_Qneed_ht+v_Qloss_ht_dist;  %Q_heat_nd for DH
             else
             v_Qht_sys =(v_Qloss_ht_dist+v_Qneed_ht)/(In.heat_sys_eff+eps);  % total heating energy need from our system including losses
             v_Qht_DH = zeros(12,1);
             end

             if DC_YesNo==1
             v_Qcl_sys = zeros(12,1);  % if we have district cooling our cooling energy needs from our system are zero
             v_Qcool_DC = v_Qloss_cl_dist+v_Qneed_cl;  % if we have DC the cooling needs are the dist losses + the cooling needs themselves
             else
             v_Qcl_sys =(v_Qloss_cl_dist+v_Qneed_cl)/(IEER+eps);  % if no DC compute our total system cooling energy needs including losses
             v_Qcool_DC=zeros(12,1); % if no DC, DC cooling needs are zero
             end


             */
            Vector v_Qcl_DC_elec = div(mult(v_Qcool_DC, 1 - cooling.eta_DC_frac_abs()), cooling.eta_DC_COP() * cooling.eta_DC_network());
            Vector v_Qcl_DC_abs = div(mult(v_Qcool_DC, 1 - cooling.frac_DC_free()), cooling.eta_DC_COP_abs());
            printVector("v_Qcl_DC_elec", v_Qcl_DC_elec);
            printVector("v_Qcl_DC_abs", v_Qcl_DC_abs);

            Vector v_Qht_DH_total = div(mult(v_Qht_DH, 1 - heating.frac_DH_free()), heating.eta_DH_sys() * heating.eta_DH_network());
            v_Qcl_elec_tot = sum(v_Qcl_sys, v_Qcl_DC_elec);
            v_Qcl_gas_tot = v_Qcl_DC_abs;
            printVector("v_Qht_DH_total", v_Qht_DH_total);
            printVector("v_Qcl_elec_tot", v_Qcl_elec_tot);
            printVector("v_Qcl_gas_tot", v_Qcl_gas_tot);

            //Vector v_Qelec_ht,v_Qgas_ht;

            if (heating.energyType() == 1) {
                v_Qelec_ht = v_Qht_sys;
                v_Qgas_ht = v_Qht_DH_total;
            }
            else {
                v_Qelec_ht = Vector(12);
                zero(v_Qelec_ht);
                v_Qgas_ht = sum(v_Qht_sys, v_Qht_DH_total);
            }
            printVector("v_Qelec_ht", v_Qelec_ht);
            printVector("v_Qgas_ht", v_Qgas_ht);

            // From original matlab code. Preserved for future implementation of district heating/cooling. BAA@2015-07-15.
            /*
             v_Qcl_DC_elec = v_Qcool_DC * (1-n_eta_DC_frac_abs) / (n_eta_DC_COP*n_eta_DC_network);  % Energy used for cooling by district electric chillers
             v_Qcl_DC_abs =  v_Qcool_DC * (1-n_frac_DC_free) / n_eta_DC_COP_abs; %Energy used for cooling by district absorption chillers

             v_Qht_DH_total = v_Qht_DH * (1 - n_frac_DH_free) / (n_eta_DH_sys * n_eta_DH_network);
             v_Qcl_elec_tot = v_Qcl_sys + v_Qcl_DC_elec; %total electric cooling energy (MJ)
             v_Qcl_gas_tot = v_Qcl_DC_abs; % total gas cooliing energy

             if In.heat_energy_type==1  %check if fuel type is electric
             v_Qelec_ht=v_Qht_sys;  % total electric heating energy (MJ)
             v_Qgas_ht=v_Qht_DH_total; % total gas heating energy is DH if fuel type is electric
             else
             v_Qelec_ht = zeros(12,1);  % if we get here, fuel was gas to total electric heating energy is 0
             v_Qgas_ht=v_Qht_sys+v_Qht_DH_total;  % total gas heating energy is building + any DH
             end

             */
        }

        /**
         * Calculate energy for pumps used in the heating/cooling systems.
         * References: EPA NR 6.9.7.1 and 6.9.7.2, EN 15243.
         */
        void MonthlyModel::pump(const Vector& v_Qneed_ht, const Vector& v_Qneed_cl, double Qneed_ht_yr, double Qneed_cl_yr, Vector& v_Q_pump_tot) const
        {
            // TODO: The current implementation is wrong. It either needs to be revised to be more like the hourly implementation where the pump energy
            // is multiplied by the amount of time the pumps are actually on or heating.E_pumps()/cooling.E_pumps() needs to be expressed in terms of the
            // heating/cooling delivered so that the pump energy can be determined by multiplying it by the heating/cooling delivered. Both methods
            // have challenges, which is why they are not yet implements. Until then, consider the monthly pump values unreliable. BAA@2015-07-15.

            // Total annual pump energy for heating systems if the pumps are running continuously.
            // NOTE: This assumption (that the annual pump energy is equal to the energy of the pumps running continuosly) is the source of the
            // problems in the pump results. BAA@2015-07-15.
            double Q_pumps_yr_ht = sum(mult(megasecondsInMonth, heating.E_pumps(), 12));
            // Total annual pump energy for cooling systems if the pumps are running continuously.
            double Q_pumps_yr_cl = sum(mult(megasecondsInMonth, cooling.E_pumps(), 12));

            // Fraction of time the system is in heating mode each month.
            Vector v_frac_ht_mode = div(v_Qneed_ht, sum(v_Qneed_ht, v_Qneed_cl));
            // Total heating energy fraction.
            double frac_ht_total = sum(v_frac_ht_mode);
            // Total yearly pump energy.
            double Q_pumps_ht = Q_pumps_yr_ht * heating.pumpControlReduction() * structure.floorArea();
            // Distribute the total annual pump energy between the 12 months proportional to the distribution of the heating
            Vector v_Q_pumps_ht = div(mult(v_frac_ht_mode, Q_pumps_ht), frac_ht_total);

            // Fraction of time the system is in cooling mode each month.
            Vector v_frac_cl_mode = div(v_Qneed_cl, sum(v_Qneed_ht, v_Qneed_cl));
            // Total cooling energy fraction.
            double frac_cl_total = sum(v_frac_cl_mode);
            // Total yearly pump energy.
            double Q_pumps_cl = Q_pumps_yr_cl * cooling.pumpControlReduction() * structure.floorArea();
            // Distribute the total annual pump energy between the 12 months proportional to the distribution of the cooling.
            Vector v_Q_pumps_cl = div(mult(v_frac_cl_mode, Q_pumps_cl), frac_cl_total);

            // Total pump operational factor.
            Vector v_frac_tot = div(sum(v_Qneed_ht, v_Qneed_cl), Qneed_ht_yr + Qneed_cl_yr);
            double frac_total = sum(v_frac_tot);
            double Q_pumps_tot = Q_pumps_ht + Q_pumps_cl;

            if (Q_pumps_ht == 0 || Q_pumps_cl == 0) {
                // If there is just heating or just cooling, use the individual heating or cooling pump energy vector.
                v_Q_pump_tot = sum(v_Q_pumps_ht, v_Q_pumps_cl);
            }
            else {
                // Otherwise, distribut the combined pump energy proportional to the combined heating/cooling load.
                v_Q_pump_tot = div(mult(v_frac_tot, Q_pumps_tot), frac_total);
            }
        }

        /**
         * Energy Generation
         * NOT INCLUDED YET
         */
        void MonthlyModel::energyGeneration() const
        {
        }

        /**
         * Calculate domestic hot water (DHW).
         * References: NEN 2916 12.2
         */
        void MonthlyModel::heatedWater(Vector& v_Q_dhw_elec, Vector& v_Q_dhw_gas) const
        {
            // Energy from solar energy hot water collectors - not included yet
            Vector v_Q_dhw_solar(12);
            zero(v_Q_dhw_solar);

            // Total annual energy demand required for heating DHW (MJ/yr).
            double Q_dhw_yr = heating.hotWaterDemand() * (heating.dhw_tset() - heating.dhw_tsupply()) * phys.rhoCpWater();

            Vector v_MonthlyDemand = mult(daysInMonth, Q_dhw_yr, 12);
            Vector v_frac_MonthlyDemand_yr = div(v_MonthlyDemand, daysInYear);
            Vector v_Qe_demand = div(v_frac_MonthlyDemand_yr, heating.hotWaterDistributionEfficiency());

            // Monthly DHW energy demand including distribution efficiency.
            Vector v_Q_dhw_demand = div(v_Qe_demand, kWh2MJ);
            // Total monthly supply need is (demand - solar)/system efficiency.
            Vector v_Q_dhw_need = maximum(div(dif(v_Q_dhw_demand, v_Q_dhw_solar), heating.hotWaterSystemEfficiency()), 0);

            // Vector of zeroes for fuel type that is unused.
            Vector Z(v_Q_dhw_need.size(), 0.0);

            printVector("v_MonthlyDemand", v_MonthlyDemand);
            printVector("v_frac_MonthlyDemand_yr", v_frac_MonthlyDemand_yr);
            printVector("v_Qe_demand", v_Qe_demand);
            printVector("v_Q_dhw_demand", v_Q_dhw_demand);
            printVector("v_Q_dhw_need", v_Q_dhw_need);
            printVector("Z", Z);

            if (heating.hotWaterEnergyType() == 1) {
                v_Q_dhw_elec = v_Q_dhw_need;
                v_Q_dhw_gas = Z;
            }
            else {
                v_Q_dhw_gas = v_Q_dhw_need;
                v_Q_dhw_elec = Z;
            }
            printVector("v_Q_dhw_gas", v_Q_dhw_gas);
            printVector("v_Q_dhw_elec", v_Q_dhw_elec);
        }

        std::vector<EndUses> MonthlyModel::simulate() const
        {
            Vector weekdayOccupiedMegaseconds(12);
            Vector weekdayUnoccupiedMegaseconds(12);
            Vector weekendOccupiedMegaseconds(12);
            Vector weekendUnoccupiedMegaseconds(12);
            Vector clockHourOccupied(24);
            Vector clockHourUnoccupied(24);
            double frac_hrs_wk_day = 0;
            double hoursUnoccupiedPerDay = 0;
            double hoursOccupiedPerDay = 0;
            double frac_hrs_wk_nt = 0;
            double frac_hrs_wke_tot = 0;

            //Solor Radiation Breakdown Results
            Vector v_hrs_sun_down_mo(12), v_Tdbt_nt, v_Tdbt_day;
            Vector frac_Pgh_wk_nt, frac_Pgh_wke_day, frac_Pgh_wke_nt;
            //Envelop Calculations Results
            Vector v_win_A, v_wall_emiss, v_wall_alpha_sc, v_wall_U, v_wall_A;

            Vector v_wall_A_sol, v_win_hr, v_wall_R_sc, v_win_A_sol;

            double Q_illum_occ, Q_illum_unocc, Q_illum_tot_yr;

            double phi_int_avg, phi_plug_avg, phi_illum_avg;

            double phi_int_wk_nt, phi_int_wke_day, phi_int_wke_nt;
            Vector v_E_sol;

            double H_tr;
            Vector v_P_tot_wke_day, v_P_tot_wk_nt, v_P_tot_wke_nt;

            Vector v_Th_avg(12), v_Tc_avg(12);

            double phi_I_tot, tau;
            Vector v_Hve_ht, v_Hve_cl;

            double Qneed_ht_yr, Qneed_cl_yr;
            Vector v_Qneed_ht, v_Qneed_cl;

            Vector v_Qelec_ht, v_Qcl_elec_tot, v_Q_illum_tot, v_Q_illum_ext_tot, v_Qfan_tot, v_Q_pump_tot, v_Q_dhw_elec, v_Qgas_ht, v_Qcl_gas_tot, v_Q_dhw_gas;

            frac_hrs_wk_day = hoursUnoccupiedPerDay = hoursOccupiedPerDay = frac_hrs_wk_nt = frac_hrs_wke_tot = 1;

            //openstudio::isomodel::loadDefaults(monthlyModel);

            if (DEBUG_ISO_MODEL_SIMULATION)
                std::cout << std::endl << "scheduleAndOccupancy: " << std::endl;
            scheduleAndOccupancy(weekdayOccupiedMegaseconds, weekdayUnoccupiedMegaseconds, weekendOccupiedMegaseconds, weekendUnoccupiedMegaseconds,
                clockHourOccupied, clockHourUnoccupied, frac_hrs_wk_day, hoursUnoccupiedPerDay, hoursOccupiedPerDay, frac_hrs_wk_nt, frac_hrs_wke_tot);

            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "frac_hrs_wk_day: " << frac_hrs_wk_day << std::endl;
                std::cout << "hoursUnoccupiedPerDay: " << hoursUnoccupiedPerDay << std::endl;
                std::cout << "hoursOccupiedPerDay: " << hoursOccupiedPerDay << std::endl;
                std::cout << "frac_hrs_wk_nt: " << frac_hrs_wk_nt << std::endl;
                std::cout << "frac_hrs_wke_tot: " << frac_hrs_wke_tot << std::endl;

                printVector("weekdayOccupiedMegaseconds", weekdayOccupiedMegaseconds);
                printVector("weekdayUnoccupiedMegaseconds", weekdayUnoccupiedMegaseconds);
                printVector("weekendOccupiedMegaseconds", weekendOccupiedMegaseconds);
                printVector("weekendUnoccupiedMegaseconds", weekendUnoccupiedMegaseconds);
                printVector("clockHourOccupied", clockHourOccupied);
                printVector("clockHourUnoccupied", clockHourUnoccupied);

                std::cout << std::endl << "solarRadiationBreakdown: " << std::endl;
            }
            solarRadiationBreakdown(weekdayOccupiedMegaseconds, weekdayUnoccupiedMegaseconds, weekendOccupiedMegaseconds, weekendUnoccupiedMegaseconds,
                clockHourOccupied, clockHourUnoccupied, v_hrs_sun_down_mo, frac_Pgh_wk_nt, frac_Pgh_wke_day, frac_Pgh_wke_nt, v_Tdbt_nt, v_Tdbt_day);

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_hrs_sun_down_mo", v_hrs_sun_down_mo);
                printVector("frac_Pgh_wk_nt", frac_Pgh_wk_nt);
                printVector("frac_Pgh_wke_day", frac_Pgh_wke_day);
                printVector("frac_Pgh_wke_nt", frac_Pgh_wke_nt);
                printVector("v_Tdbt_nt", v_Tdbt_nt);
                printVector("v_Tdbt_day", v_Tdbt_day);

                std::cout << std::endl << "lightingEnergyUse: " << std::endl;
            }
            lightingEnergyUse(v_hrs_sun_down_mo, Q_illum_occ, Q_illum_unocc, Q_illum_tot_yr, v_Q_illum_tot, v_Q_illum_ext_tot);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "Q_illum_occ: " << Q_illum_occ << std::endl;
                std::cout << "Q_illum_unocc: " << Q_illum_unocc << std::endl;
                std::cout << "Q_illum_unocc: " << Q_illum_unocc << std::endl;
                printVector("v_Q_illum_tot", v_Q_illum_tot);
                printVector("v_Q_illum_ext_tot", v_Q_illum_ext_tot);

                std::cout << std::endl << "envelopCalculations: " << std::endl;/*
                 v_wall_A = structure.wallArea();
                 v_win_A = structure.windowArea();
                 v_wall_U = structure.wallUniform();
                 Vector v_win_U = structure.windowUniform();*/
                printVector("structure.wallArea()", structure.wallArea());
                printVector("structure.windowArea()", structure.windowArea());
                printVector("structure.wallUniform()", structure.wallUniform());
                printVector("structure.windowUniform()", structure.windowUniform());
            }
            envelopCalculations(v_win_A, v_wall_emiss, v_wall_alpha_sc, v_wall_U, v_wall_A, H_tr);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "H_tr: " << H_tr << std::endl;
                printVector("v_win_A", v_win_A);
                printVector("v_wall_emiss", v_wall_emiss);
                printVector("v_wall_alpha_sc", v_wall_alpha_sc);
                printVector("v_wall_U", v_wall_U);
                printVector("v_wall_A", v_wall_A);

                std::cout << std::endl << "windowSolarGain: " << std::endl;
            }
            windowSolarGain(v_win_A, v_wall_emiss, v_wall_alpha_sc, v_wall_U, v_wall_A, v_wall_A_sol, v_win_hr, v_wall_R_sc, v_win_A_sol);

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_wall_A_sol", v_wall_A_sol);
                printVector("v_win_hr", v_win_hr);
                printVector("v_wall_R_sc", v_wall_R_sc);
                printVector("v_win_A_sol", v_win_A_sol);

                std::cout << std::endl << "solarHeatGain: " << std::endl;
            }
            solarHeatGain(v_win_A_sol, v_wall_R_sc, v_wall_U, v_wall_A, v_win_hr, v_wall_A_sol, v_E_sol);

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_E_sol", v_E_sol);

                std::cout << std::endl << "heatGainsAndLosses: " << std::endl;
            }
            heatGainsAndLosses(frac_hrs_wk_day, Q_illum_occ, Q_illum_unocc, Q_illum_tot_yr, phi_int_avg, phi_plug_avg, phi_illum_avg, phi_int_wke_nt,
                phi_int_wke_day, phi_int_wk_nt);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "phi_int_avg: " << phi_int_avg << std::endl;
                std::cout << "phi_plug_avg: " << phi_plug_avg << std::endl;
                std::cout << "phi_illum_avg: " << phi_illum_avg << std::endl;
                std::cout << "phi_int_wke_nt: " << phi_int_wke_nt << std::endl;
                std::cout << "phi_int_wke_day: " << phi_int_wke_day << std::endl;
                std::cout << "phi_int_wk_nt: " << phi_int_wk_nt << std::endl;

                std::cout << std::endl << "internalHeatGain: " << std::endl;
            }
            internalHeatGain(phi_int_avg, phi_plug_avg, phi_illum_avg, phi_I_tot);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "phi_I_tot: " << phi_I_tot << std::endl;

                std::cout << std::endl << "unoccupiedHeatGain: " << std::endl;
            }
            unoccupiedHeatGain(phi_int_wk_nt, phi_int_wke_day, phi_int_wke_nt, weekdayUnoccupiedMegaseconds, weekendOccupiedMegaseconds,
                weekendUnoccupiedMegaseconds, frac_Pgh_wk_nt, frac_Pgh_wke_day, frac_Pgh_wke_nt, v_E_sol, v_P_tot_wke_day, v_P_tot_wk_nt, v_P_tot_wke_nt);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_P_tot_wke_day", v_P_tot_wke_day);
                printVector("v_P_tot_wk_nt", v_P_tot_wk_nt);
                printVector("v_P_tot_wke_nt", v_P_tot_wke_nt);

                std::cout << std::endl << "interiorTemp: " << std::endl;
            }
            interiorTemp(v_wall_A, v_P_tot_wke_day, v_P_tot_wk_nt, v_P_tot_wke_nt, v_Tdbt_nt, v_Tdbt_day, H_tr, hoursUnoccupiedPerDay, hoursOccupiedPerDay, frac_hrs_wk_day,
                frac_hrs_wk_nt, frac_hrs_wke_tot, v_Th_avg, v_Tc_avg, tau);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "tau: " << tau << std::endl;
                printVector("v_Th_avg", v_Th_avg);
                printVector("v_Tc_avg", v_Tc_avg);

                std::cout << std::endl << "ventilationCalc: " << std::endl;
            }
            ventilationCalc(v_Th_avg, v_Tc_avg, frac_hrs_wk_day, v_Hve_ht, v_Hve_cl);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Hve_ht", v_Hve_ht);
                printVector("v_Hve_cl", v_Hve_cl);

                std::cout << std::endl << "heatingAndCooling: " << std::endl;
            }
            heatingAndCooling(v_E_sol, v_Th_avg, v_Hve_ht, v_Tc_avg, v_Hve_cl, tau, H_tr, phi_I_tot, frac_hrs_wk_day, v_Qfan_tot, v_Qneed_ht, v_Qneed_cl,
                Qneed_ht_yr, Qneed_cl_yr);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << "Qneed_ht_yr: " << Qneed_ht_yr << std::endl;
                std::cout << "Qneed_cl_yr: " << Qneed_cl_yr << std::endl;
                printVector("v_Qfan_tot", v_Qfan_tot);

                std::cout << std::endl << "hvac: " << std::endl;
            }
            hvac(v_Qneed_ht, v_Qneed_cl, Qneed_ht_yr, Qneed_cl_yr, v_Qelec_ht, v_Qgas_ht, v_Qcl_elec_tot, v_Qcl_gas_tot);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Qelec_ht", v_Qelec_ht);
                printVector("v_Qgas_ht", v_Qgas_ht);
                printVector("v_Qcl_elec_tot", v_Qcl_elec_tot);
                printVector("v_Qcl_gas_tot", v_Qcl_gas_tot);

                std::cout << std::endl << "pump: " << std::endl;
            }
            pump(v_Qneed_ht, v_Qneed_cl, Qneed_ht_yr, Qneed_cl_yr, v_Q_pump_tot);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Q_pump_tot", v_Q_pump_tot);

                std::cout << std::endl << "energyGeneration: " << std::endl;
            }
            energyGeneration();
            if (DEBUG_ISO_MODEL_SIMULATION) {
                std::cout << std::endl << "heatedWater: " << std::endl;
            }
            heatedWater(v_Q_dhw_elec, v_Q_dhw_gas);
            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Q_dhw_elec", v_Q_dhw_elec);
                printVector("v_Q_dhw_gas", v_Q_dhw_gas);
            }

            return outputGeneration(v_Qelec_ht, v_Qcl_elec_tot, v_Q_illum_tot, v_Q_illum_ext_tot, v_Qfan_tot, v_Q_pump_tot, v_Q_dhw_elec, v_Qgas_ht,
                v_Qcl_gas_tot, v_Q_dhw_gas, frac_hrs_wk_day);
        }
        std::vector<EndUses> MonthlyModel::outputGeneration(const Vector& v_Qelec_ht, const Vector& v_Qcl_elec_tot, const Vector& v_Q_illum_tot,
            const Vector& v_Q_illum_ext_tot, const Vector& v_Qfan_tot, const Vector& v_Q_pump_tot, const Vector& v_Q_dhw_elec, const Vector& v_Qgas_ht,
            const Vector& v_Qcl_gas_tot, const Vector& v_Q_dhw_gas, double frac_hrs_wk_day) const
        {
            std::vector<EndUses> allResults;

            // TODO: Move the plug load calcs to a separate function. BAA@2015-07-15

            // Average electric plug loads (W/m2).
            double E_plug_elec = building.electricApplianceHeatGainOccupied() * frac_hrs_wk_day
                + building.electricApplianceHeatGainUnoccupied() * (1.0 - frac_hrs_wk_day);
            // Average gas plug loads (W/m2).
            double E_plug_gas = building.gasApplianceHeatGainOccupied() * frac_hrs_wk_day
                + building.gasApplianceHeatGainUnoccupied() * (1.0 - frac_hrs_wk_day);

            // Electric plug load (kWh/m2).
            Vector v_Q_plug_elec = div(mult(hoursInMonth, E_plug_elec, 12), 1000.0);
            // Gas plug load (kWh/m2).
            Vector v_Q_plug_gas = div(mult(hoursInMonth, E_plug_gas, 12), 1000.0);
            printVector("v_Q_plug_elec", v_Q_plug_elec);
            printVector("v_Q_plug_gas", v_Q_plug_gas);

            // Electric loads (kWh/m2).
            Vector Eelec_ht = div(div(v_Qelec_ht, structure.floorArea()), kWh2MJ); // Total monthly electric usage for heating.
            Vector Eelec_cl = div(div(v_Qcl_elec_tot, structure.floorArea()), kWh2MJ); // Total monthly electric usage for cooling.
            Vector Eelec_int_lt = div(v_Q_illum_tot, structure.floorArea()); // Total monthly electric usage density for interior lighting.
            Vector Eelec_ext_lt = div(v_Q_illum_ext_tot, structure.floorArea()); // Total monthly electric usage for exterior lights.
            Vector Eelec_fan = v_Qfan_tot; // Total monthly elec usage for fans.
            Vector Eelec_pump = div(div(v_Q_pump_tot, structure.floorArea()), kWh2MJ); // Total monthly elec usage for pumps.
            Vector Eelec_plug = v_Q_plug_elec; // Total monthly elec usage for elec plugloads.
            Vector Eelec_dhw = div(v_Q_dhw_elec, structure.floorArea());

            if (DEBUG_ISO_MODEL_SIMULATION) {
                printVector("v_Qcl_elec_tot", v_Qcl_elec_tot);
                printVector("v_Q_pump_tot", v_Q_pump_tot);
                printVector("Eelec_cl", Eelec_cl);
                printVector("Eelec_pump", Eelec_pump);
                std::cout << "floorArea: " << structure.floorArea() << std::endl;
            }

            // Gas loads (kWh/m2).
            Vector Egas_ht = div(div(v_Qgas_ht, structure.floorArea()), kWh2MJ); // Total monthly gas usage for heating.
            Vector Egas_cl = div(div(v_Qcl_gas_tot, structure.floorArea()), kWh2MJ); // Total monthly gas usage for cooling.
            Vector Egas_plug = v_Q_plug_gas; // Total monthly gas plugloads.
            Vector Egas_dhw = div(v_Q_dhw_gas, structure.floorArea()); // Total monthly dhw gas plugloads.

            EndUses results[12];
            for (int i = 0; i < 12; i++) {

#ifdef ISOMODEL_STANDALONE
                int euse = 0;
                results[i].addEndUse(euse++, Eelec_ht[i]);
                results[i].addEndUse(euse++, Eelec_cl[i]);
                results[i].addEndUse(euse++, Eelec_int_lt[i]);
                results[i].addEndUse(euse++, Eelec_ext_lt[i]);
                results[i].addEndUse(euse++, Eelec_fan[i]);
                results[i].addEndUse(euse++, Eelec_pump[i]);
                results[i].addEndUse(euse++, Eelec_plug[i]);
                results[i].addEndUse(euse++, 0); // Exterior Equipment
                results[i].addEndUse(euse++, Eelec_dhw[i]);
                results[i].addEndUse(euse++, Egas_ht[i]);
                results[i].addEndUse(euse++, Egas_cl[i]);
                results[i].addEndUse(euse++, Egas_plug[i]);
                results[i].addEndUse(euse++, Egas_dhw[i]);
#else
                results[i].addEndUse(Eelec_ht[i], EndUseFuelType::Electricity, EndUseCategoryType::Heating);
                results[i].addEndUse(Eelec_cl[i], EndUseFuelType::Electricity, EndUseCategoryType::Cooling);
                results[i].addEndUse(Eelec_int_lt[i], EndUseFuelType::Electricity, EndUseCategoryType::InteriorLights);
                results[i].addEndUse(Eelec_ext_lt[i], EndUseFuelType::Electricity, EndUseCategoryType::ExteriorLights);
                results[i].addEndUse(Eelec_fan[i], EndUseFuelType::Electricity, EndUseCategoryType::Fans);
                results[i].addEndUse(Eelec_pump[i], EndUseFuelType::Electricity, EndUseCategoryType::Pumps);
                results[i].addEndUse(Eelec_plug[i], EndUseFuelType::Electricity, EndUseCategoryType::InteriorEquipment);
                results[i].addEndUse(0, EndUseFuelType::Electricity, EndUseCategoryType::ExteriorEquipment);
                results[i].addEndUse(Eelec_dhw[i], EndUseFuelType::Electricity, EndUseCategoryType::WaterSystems);

                results[i].addEndUse(Egas_ht[i], EndUseFuelType::Gas, EndUseCategoryType::Heating);
                results[i].addEndUse(Egas_cl[i], EndUseFuelType::Gas, EndUseCategoryType::Cooling);
                results[i].addEndUse(Egas_plug[i], EndUseFuelType::Gas, EndUseCategoryType::InteriorEquipment);
                results[i].addEndUse(Egas_dhw[i], EndUseFuelType::Gas, EndUseCategoryType::WaterSystems);
#endif
                allResults.push_back(results[i]);
            }
            return allResults;

            // TODO: Why is this here? It is after the function returns... BAA@2015-07-15.

            // Calculate the annual totals.
            Vector Etot_ht = sum(Eelec_ht, Egas_ht);
            Vector Etot_cl = sum(Eelec_cl, Egas_cl);
            Vector Etot_int_lt = Eelec_int_lt; // Total monthly electric usage density for interior lighting
            Vector Etot_ext_lt = Eelec_ext_lt; // Total monthly electric usage for exterior lights
            Vector Etot_fan = Eelec_fan; // Total monthly elec usage for fans
            Vector Etot_pump = Eelec_pump; // Total monthly elec usage for pumps
            Vector Etot_plug = sum(v_Q_plug_elec, v_Q_plug_gas); // Total monthly elec usage for elec plugloads
            Vector Etot_dhw = sum(v_Q_dhw_elec, v_Q_plug_elec);

            // Find the total annual energy use.
            double yrSum = 0;
            Vector monthly(Etot_ht.size());
            for (unsigned int i = 0; i < Etot_ht.size(); i++) {
                monthly[i] = Etot_ht[i] + Etot_cl[i] + Etot_int_lt[i] + Etot_ext_lt[i] + Etot_fan[i] + Etot_pump[i] + Etot_plug[i] + Etot_dhw[i];
                yrSum += monthly[i];
            }
        }
    } // isomodel
} // openstudio