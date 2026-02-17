/// @file TimeFrame.cpp
/// @brief Hour-of-year to month, day-of-week, and hour-of-day conversion utility.
///
/// Pre-computes lookup tables for converting a linear hour index (0-8759)
/// to month (0-11), day of month, day of week (0-6), and hour of day
/// (0-23). Used by HourlyModel and SolarRadiation for time indexing.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#include "TimeFrame.hpp"

#include "Constants.hpp"

namespace openstudio::isomodel {

TimeFrame::TimeFrame() {
  int hourOfYear = 0;
  int dayOfYear = 0;
  int dayOfWeek = 0;
  int dim;

  for (int month = 1; month <= 12; month++) {
    dim = monthLength(month);
    for (int dayOfMonth = 1; dayOfMonth <= dim; dayOfMonth++) {
      for (int hourOfDay = 0; hourOfDay <= 23; hourOfDay++) {
        Hour[hourOfYear] = hourOfDay;
        DayOfMonth[hourOfYear] = dayOfMonth;
        DayOfWeek[hourOfYear] = dayOfWeek;
        Month[hourOfYear] = month;
        YTD[hourOfYear] = dayOfYear;
        ++hourOfYear;
      }
      ++dayOfYear;
      dayOfWeek = (dayOfWeek == 6) ? 0 : dayOfWeek + 1;
    }
  }
}

// Fix: Explicitly define the destructor here
TimeFrame::~TimeFrame() = default;

int TimeFrame::monthLength(int month) {
  if (month < 1 || month > 12)
    return 0;
  return static_cast<int>(DAYS_IN_MONTH[month - 1]);
}

} // namespace openstudio::isomodel