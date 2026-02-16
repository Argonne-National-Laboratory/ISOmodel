// C++ Schedules.hpp
#pragma once
#include "ISOModelAPI.hpp" // For ISOMODEL_API macro
#include "MathHelpers.hpp"

#include <string>
#include <vector>

// Forward declarations for parameters in buildWeeklySchedules
namespace openstudio::isomodel {
class Population;
class Ventilation;
class Building;
class Lighting;
class Heating;
class Cooling;
} // namespace openstudio::isomodel

namespace openstudio::isomodel::schedules {

// Struct to hold raw CSV schedule data (moved out of HourlyModel)
struct LoadedScheduleData final {
  int Hour;
  double MechVent;
  double IntApp;
  double IntLight;
  int ExtLight;
  int ExtEquip;
  int HeatSet;
  int CoolSet;
};

// Weekly schedule container (moved out of HourlyModel)
struct WeeklyScheduleData final {
  double q_ve[24][7];
  double ext_App[24][7];
  double int_App[24][7];
  double ext_L[24][7];
  double int_L[24][7];
  double theta_H[24][7]; // Heating setpoint
  double theta_C[24][7]; // Cooling setpoint
};

// NEW: Struct to hold only the schedule-related data for HourlyCache
struct ScheduleDataForHourlyCache final {
  float sched_q_ve_mech = 0.0f;   // Mechanical ventilation schedule
  float sched_phi_int_App = 0.0f; // Appliances gain schedule
  float sched_phi_int_L = 0.0f;   // Lighting gain schedule
  float sched_ext_light = 0.0f;   // Exterior lighting control
  float sched_ext_equip = 0.0f;   // Exterior equipment control
  float sched_theta_H_set = 0.0f; // Heating setpoint
  float sched_theta_C_set = 0.0f; // Cooling setpoint
};

// NEW: Struct to hold schedule and occupancy data for MonthlyModel
struct MonthlyScheduleData final {
  Vector weekdayOccupiedMegaseconds;
  Vector weekdayUnoccupiedMegaseconds;
  Vector weekendOccupiedMegaseconds;
  Vector weekendUnoccupiedMegaseconds;
  Vector clockHourOccupied;
  Vector clockHourUnoccupied;
  double frac_hrs_wk_day = 0.0;
  double hoursUnoccupiedPerDay = 0.0;
  double hoursOccupiedPerDay = 0.0;
  double frac_hrs_wk_nt = 0.0;
  double frac_hrs_wke_tot = 0.0;
};

/**
 * Try to load hourly schedules from a CSV file.
 * Returns true on success and fills 'data' with 8760 rows (or resized/resampled as original).
 */
ISOMODEL_API bool loadHourlySchedulesFromFile(const std::string &path,
                                              std::vector<LoadedScheduleData> &data);

/**
 * Build the weekly schedules (24 x 7 arrays) from the given model inputs.
 * This mirrors the original HourlyModel::buildWeeklySchedules implementation
 * but takes the necessary objects as parameters so code can live outside HourlyModel.
 */
ISOMODEL_API void buildWeeklySchedules(const openstudio::isomodel::Population &pop,
                                       const openstudio::isomodel::Ventilation &ventilation,
                                       const openstudio::isomodel::Building &building,
                                       const openstudio::isomodel::Lighting &lights,
                                       const openstudio::isomodel::Heating &heating,
                                       const openstudio::isomodel::Cooling &cooling,
                                       WeeklyScheduleData &sched);

/**
 * Orchestrates the loading or generation of hourly schedule data.
 * Returns a vector of ScheduleDataForHourlyCache, which HourlyModel can then use
 * to populate its HourlyCache.
 */
ISOMODEL_API std::vector<ScheduleDataForHourlyCache> getHourlySchedules(
    const std::string &hourlySchedulePath, const openstudio::isomodel::Population &pop,
    const openstudio::isomodel::Ventilation &ventilation,
    const openstudio::isomodel::Building &building, const openstudio::isomodel::Lighting &lights,
    const openstudio::isomodel::Heating &heating, const openstudio::isomodel::Cooling &cooling);

/**
 * Generates schedule and occupancy data for the MonthlyModel.
 */
ISOMODEL_API MonthlyScheduleData getMonthlySchedules(const openstudio::isomodel::Population &pop);

} // namespace openstudio::isomodel::schedules

