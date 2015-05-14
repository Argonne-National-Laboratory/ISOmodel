#include "TimeFrame.hpp"

namespace openstudio {
namespace isomodel {
TimeFrame::TimeFrame(void)
{
  int hourOfYear = 0;
  int dayOfYear = 0;
  int dayOfWeek = 0;
  int dim;

  for (int month = 1; month <= 12; month++) {
    dim = monthLength(month);
    for (int dayOfMonth = 1; dayOfMonth <= dim; dayOfMonth++) {
      for (int hourOfDay = 0; hourOfDay <= 23; hourOfDay++) {
        this->Hour[hourOfYear] = hourOfDay;
        this->DayOfMonth[hourOfYear] = dayOfMonth;
        this->DayOfWeek[hourOfYear] = dayOfWeek;
        this->Month[hourOfYear] = month;
        this->YTD[hourOfYear] = dayOfYear;
        ++hourOfYear;
      }
      ++dayOfYear;
      dayOfWeek = (dayOfWeek == 6) ? 0 : dayOfWeek + 1;
    }
  }

}

TimeFrame::~TimeFrame(void)
{

}

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
}
}
