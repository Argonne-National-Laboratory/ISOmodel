/// @file Schedules.cpp
/// @brief Hourly and weekly schedule generation and CSV loading.
///
/// Builds 24×7 weekly schedule arrays for ventilation, appliances, lighting,
/// and temperature setpoints from Population and Building properties.
/// Optionally loads custom hourly schedules from CSV files. Generates
/// monthly occupancy fractions for the MonthlyModel.
///
/// @author Ralph Muehleisen
/// @date 2026-01-23
/// @copyright Copyright Argonne National Laboratory
#include "Schedules.hpp"

#include "Building.hpp"
#include "Constants.hpp" // For HOURS_IN_DAY, DAYS_IN_WEEK, HOURS_IN_YEAR
#include "Cooling.hpp"
#include "Heating.hpp"
#include "Lighting.hpp"
#include "Population.hpp"
#include "Profiler.hpp"
#include "TimeFrame.hpp" // Needed for TimeFrame to map weekly to hourly
#include "Ventilation.hpp"

#include <algorithm> // For std::max
#include <fstream>
#include <iostream>
#include <sstream>

namespace openstudio::isomodel::schedules {

bool loadHourlySchedulesFromFile(const std::string &path, std::vector<LoadedScheduleData> &data) {
  PROFILE_FUNCTION();
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open schedule file: " << path << std::endl;
    return false;
  }

  std::string line;
  std::getline(file, line); // Skip header

  data.clear();
  data.reserve(HOURS_IN_YEAR); // Use HOURS_IN_YEAR from Constants.hpp

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string cell;
    LoadedScheduleData row;

    try {
      // Expected format:
      // Hour,MechVent,IntApp,IntLight,ExtLight,ExtEquip,HeatSet,CoolSet
      std::getline(ss, cell, ',');
      row.Hour = std::stoi(cell);
      std::getline(ss, cell, ',');
      row.MechVent = std::stod(cell);
      std::getline(ss, cell, ',');
      row.IntApp = std::stod(cell);
      std::getline(ss, cell, ',');
      row.IntLight = std::stod(cell);
      std::getline(ss, cell, ',');
      row.ExtLight = std::stoi(cell);
      std::getline(ss, cell, ',');
      row.ExtEquip = std::stoi(cell);
      std::getline(ss, cell, ',');
      row.HeatSet = std::stoi(cell);
      std::getline(ss, cell, ',');
      row.CoolSet = std::stoi(cell);
      data.push_back(row);
    } catch (...) {
      std::cerr << "Error parsing line in schedule file: " << line << std::endl;
      return false;
    }
  }

  if (data.size() < HOURS_IN_YEAR) { // Use HOURS_IN_YEAR from Constants.hpp
    std::cerr << "Warning: Schedule file has fewer than " << HOURS_IN_YEAR << " rows ("
              << data.size() << ")" << std::endl;
    // Could fill remainder or fail. For now, let's warn.
    // To prevent crash in loop, resize with defaults or last value
    if (data.empty())
      return false;
    data.resize(HOURS_IN_YEAR, data.back());
  }

  return true;
}

void buildWeeklySchedules(const openstudio::isomodel::Population &pop,
                          const openstudio::isomodel::Ventilation &ventilation,
                          const openstudio::isomodel::Building &building,
                          const openstudio::isomodel::Lighting &lights,
                          const openstudio::isomodel::Heating &heating,
                          const openstudio::isomodel::Cooling &cooling, WeeklyScheduleData &sched) {
  PROFILE_FUNCTION();
  const int dayStart = static_cast<int>(pop.daysStart()), dayEnd = static_cast<int>(pop.daysEnd());
  const int hourStart = static_cast<int>(pop.hoursStart()),
            hourEnd = static_cast<int>(pop.hoursEnd());
  const double ventRate = ventilation.supplyRate(), extEquip = building.externalEquipment();
  const double intOcc = building.electricApplianceHeatGainOccupied(),
               intUnocc = building.electricApplianceHeatGainUnoccupied();
  const double intLtOcc = lights.powerDensityOccupied(),
               intLtUnocc = lights.powerDensityUnoccupied();
  const double htOcc = heating.temperatureSetPointOccupied(),
               htUnocc = heating.temperatureSetPointUnoccupied();
  const double clOcc = cooling.temperatureSetPointOccupied(),
               clUnocc = cooling.temperatureSetPointUnoccupied();

  for (int h = 0; h < HOURS_IN_DAY; ++h) {
    bool hoccupied = (h >= hourStart && h <= hourEnd);
    for (int d = 0; d < DAYS_IN_WEEK; ++d) {
      bool popoccupied = hoccupied && (d >= dayStart && d <= dayEnd);
      sched.q_ve[h][d] = hoccupied ? ventRate : 0.0;
      sched.ext_App[h][d] = extEquip;
      sched.int_App[h][d] = popoccupied ? intOcc : intUnocc;
      sched.ext_L[h][d] = 1.0;
      sched.int_L[h][d] = popoccupied ? intLtOcc : intLtUnocc;
      sched.theta_H[h][d] = popoccupied ? htOcc : htUnocc;
      sched.theta_C[h][d] = popoccupied ? clOcc : clUnocc;
    }
  }
}

std::vector<ScheduleDataForHourlyCache> getHourlySchedules(
    const std::string &hourlySchedulePath, const openstudio::isomodel::Population &pop,
    const openstudio::isomodel::Ventilation &ventilation,
    const openstudio::isomodel::Building &building, const openstudio::isomodel::Lighting &lights,
    const openstudio::isomodel::Heating &heating, const openstudio::isomodel::Cooling &cooling) {
  PROFILE_FUNCTION();

  std::vector<ScheduleDataForHourlyCache> hourlyScheduleData(HOURS_IN_YEAR);
  std::vector<LoadedScheduleData> fileData;
  bool useFile = false;

  // Try to load from file if path is present and not "false"
  if (!hourlySchedulePath.empty()) {
    std::string lowerPath = hourlySchedulePath;
    std::ranges::transform(lowerPath, lowerPath.begin(),
                           [](unsigned char c) { return std::tolower(c); });
    if (lowerPath != "false") {
      useFile = loadHourlySchedulesFromFile(hourlySchedulePath, fileData);
    }
  }

  if (useFile) {
    for (int i = 0; i < HOURS_IN_YEAR; ++i) {
      const auto &row = fileData[i];
      ScheduleDataForHourlyCache &s = hourlyScheduleData[i];
      s.sched_q_ve_mech = (float)row.MechVent;
      s.sched_ext_equip = (float)row.ExtEquip;
      s.sched_phi_int_App = (float)row.IntApp;
      s.sched_ext_light = (float)row.ExtLight;
      s.sched_phi_int_L = (float)row.IntLight;
      s.sched_theta_H_set = (float)row.HeatSet;
      s.sched_theta_C_set = (float)row.CoolSet;
    }
  } else {
    WeeklyScheduleData weekly;
    buildWeeklySchedules(pop, ventilation, building, lights, heating, cooling, weekly);

    TimeFrame frame; // Need TimeFrame to map hour-of-year to day/hour-of-week
    for (int i = 0; i < HOURS_IN_YEAR; ++i) {
      int h = frame.Hour[i];
      int d = frame.DayOfWeek[i];
      ScheduleDataForHourlyCache &s = hourlyScheduleData[i];

      s.sched_q_ve_mech = (float)weekly.q_ve[h][d];
      s.sched_ext_equip = (float)weekly.ext_App[h][d];
      s.sched_phi_int_App = (float)weekly.int_App[h][d];
      s.sched_ext_light = (float)weekly.ext_L[h][d];
      s.sched_phi_int_L = (float)weekly.int_L[h][d];
      s.sched_theta_H_set = (float)weekly.theta_H[h][d];
      s.sched_theta_C_set = (float)weekly.theta_C[h][d];
    }
  }
  return hourlyScheduleData;
}

MonthlyScheduleData getMonthlySchedules(const openstudio::isomodel::Population &pop) {
  PROFILE_FUNCTION();
  MonthlyScheduleData data;

  // Initialize vectors
  data.weekdayOccupiedMegaseconds.resize(MONTHS_IN_YEAR);
  data.weekdayUnoccupiedMegaseconds.resize(MONTHS_IN_YEAR);
  data.weekendOccupiedMegaseconds.resize(MONTHS_IN_YEAR);
  data.weekendUnoccupiedMegaseconds.resize(MONTHS_IN_YEAR);
  data.clockHourOccupied.resize(HOURS_IN_DAY);
  data.clockHourUnoccupied.resize(HOURS_IN_DAY);

  data.hoursOccupiedPerDay = pop.hoursEnd() - pop.hoursStart();
  if (data.hoursOccupiedPerDay < 0) {
    data.hoursOccupiedPerDay += HOURS_IN_DAY;
  }
  double daysOccupiedPerWeek = pop.daysEnd() - pop.daysStart() + 1;
  if (daysOccupiedPerWeek < 0) {
    daysOccupiedPerWeek += DAYS_IN_WEEK;
  }

  double hoursOccupiedDuringWeek = data.hoursOccupiedPerDay * daysOccupiedPerWeek;
  data.frac_hrs_wk_day = hoursOccupiedDuringWeek / HOURS_IN_WEEK;

  data.hoursUnoccupiedPerDay = 24 - data.hoursOccupiedPerDay;
  double hoursUnoccupiedDuringWeek = (daysOccupiedPerWeek - 1) * data.hoursUnoccupiedPerDay;
  data.frac_hrs_wk_nt = hoursUnoccupiedDuringWeek / HOURS_IN_WEEK;

  double totalWeekendHours = HOURS_IN_WEEK - hoursOccupiedDuringWeek - hoursUnoccupiedDuringWeek;
  data.frac_hrs_wke_tot = totalWeekendHours / HOURS_IN_WEEK;

  double weekendHoursOccupied = (DAYS_IN_WEEK - daysOccupiedPerWeek) * data.hoursOccupiedPerDay;
  double frac_hrs_wke_day = weekendHoursOccupied / HOURS_IN_WEEK;

  double weekendHoursUnoccupied = totalWeekendHours - weekendHoursOccupied;
  double frac_hrs_wke_nt = weekendHoursUnoccupied / HOURS_IN_WEEK;

  for (int m = 0; m < MONTHS_IN_YEAR; m++) {
    data.weekdayOccupiedMegaseconds[m] = MEGASECONDS_IN_MONTH[m] * data.frac_hrs_wk_day;
    data.weekdayUnoccupiedMegaseconds[m] = MEGASECONDS_IN_MONTH[m] * data.frac_hrs_wk_nt;
    data.weekendOccupiedMegaseconds[m] = MEGASECONDS_IN_MONTH[m] * frac_hrs_wke_day;
    data.weekendUnoccupiedMegaseconds[m] = MEGASECONDS_IN_MONTH[m] * frac_hrs_wke_nt;
  }
  for (int h = 0; h < HOURS_IN_DAY; h++) {
    if (h - WEEKDAY_START_HOUR >= 0 && h - WEEKDAY_START_HOUR < data.hoursOccupiedPerDay) {
      data.clockHourOccupied[h] = 1;
      data.clockHourUnoccupied[h] = 0;
    } else {
      data.clockHourOccupied[h] = 0;
      data.clockHourUnoccupied[h] = 1;
    }
  }
  return data;
}

} // namespace openstudio::isomodel::schedules