#include "TimeFrame.hpp"

namespace openstudio::isomodel {

TimeFrame::TimeFrame()
{
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

int TimeFrame::monthLength(int month)
{
  switch (month) {
  case 2:
    return 28;
  case 9:
  case 4:
  case 6:
  case 11:
    return 30;
  default:
    return 31;
  }
}

} // namespace openstudio::isomodel