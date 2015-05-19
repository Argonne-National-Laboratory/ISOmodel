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
#ifndef ISOMODEL_TIMEFRAME_HPP
#define ISOMODEL_TIMEFRAME_HPP

namespace openstudio {
namespace isomodel {
#define TIMESLICES 8760

/**
* Simple data structure that allows conversion from the hour of the year 
* to a variety of useful times (day of week, month, etc.).
*/
class TimeFrame
{
protected:

public:
  /// Returns the number of days in the month.
  int monthLength(int month);

  /// Returns the day of the year (0-364).
  int YTD[TIMESLICES];

  /// Returns the hour of the day (0-23).
  int Hour[TIMESLICES];

  /// Returns the day of the month (1-monthLength)
  int DayOfMonth[TIMESLICES]; // XXX: This does not appear to ever be used. BAA@2015-05-04

  /// Returns the day of the week (0-6).
  int DayOfWeek[TIMESLICES];

  /// Returns the month (1-12).
  int Month[TIMESLICES];

  TimeFrame(void);
  ~TimeFrame(void);
};
}
}
#endif
