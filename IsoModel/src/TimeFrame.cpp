// First Commit: 2013-11-05
//
// Authors:
// - Brendan Albano
// - Brian Craig
// - Nick Collier
// - Ralph Muehleisen
//
// Summary:
// This file implements the `TimeFrame` class constructor, which populates
// the lookup arrays that map an hour-of-the-year index to its
// corresponding month, day, and hour.

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