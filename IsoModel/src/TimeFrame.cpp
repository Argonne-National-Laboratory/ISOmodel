#include "TimeFrame.hpp"

namespace openstudio {
namespace isomodel {
TimeFrame::TimeFrame(void)
{
  int hourOfYear = 0, dayOfYear = 0, dim;
  for (int month = 1; month <= 12; month++) {
    dim = monthLength(month);
    for (int dayOfMonth = 1; dayOfMonth <= dim; dayOfMonth++) {
      dayOfYear++;
      for (int hourOfDay = 1; hourOfDay <= 24; hourOfDay++) {
        this->Hour[hourOfYear] = hourOfDay;
        this->Day[hourOfYear] = dayOfMonth;
        this->Month[hourOfYear] = month;
        this->YTD[hourOfYear++] = dayOfYear;
      }
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
