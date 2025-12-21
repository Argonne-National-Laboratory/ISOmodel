/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/
#ifndef ISOMODEL_TIMEFRAME_HPP
#define ISOMODEL_TIMEFRAME_HPP

#include "ISOModelAPI.hpp"

namespace openstudio::isomodel {

#define TIMESLICES 8760

/**
* Simple data structure that allows conversion from the hour of the year 
* to a variety of useful times (day of week, month, etc.).
*/
class ISOMODEL_API TimeFrame
{
public:
  // Constructor keeps implementation in cpp to populate arrays
  TimeFrame();
  
  // Fix: Declare destructor here, define in cpp to ensure symbol export
  ~TimeFrame();

  /// Returns the number of days in the month.
  int monthLength(int month);

  /// Returns the day of the year (0-364).
  int YTD[TIMESLICES];

  /// Returns the hour of the day (0-23).
  int Hour[TIMESLICES];

  /// Returns the day of the month (1-monthLength)
  int DayOfMonth[TIMESLICES];

  /// Returns the day of the week (0-6).
  int DayOfWeek[TIMESLICES];

  /// Returns the month (1-12).
  int Month[TIMESLICES];
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_TIMEFRAME_HPP