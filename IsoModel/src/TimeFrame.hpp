#pragma once
#include "Constants.hpp"
#include "ISOModelAPI.hpp"

#include <array>

namespace openstudio::isomodel {

/// Simple data structure that allows conversion from the hour of the year
/// to a variety of useful times (day of week, month, etc.).
class ISOMODEL_API TimeFrame {
public:
  // Constructor keeps implementation in cpp to populate arrays
  TimeFrame();

  // Fix: Declare destructor here, define in cpp to ensure symbol export
  ~TimeFrame();

  /// Returns the number of days in the month.
  int monthLength(int month);

  /// Returns the day of the year (0-364).
  std::array<int, HOURS_IN_YEAR> YTD{};

  /// Returns the hour of the day (0-23).
  std::array<int, HOURS_IN_YEAR> Hour{};

  /// Returns the day of the month (1-monthLength)
  std::array<int, HOURS_IN_YEAR> DayOfMonth{};

  /// Returns the day of the week (0-6).
  std::array<int, HOURS_IN_YEAR> DayOfWeek{};

  /// Returns the month (1-12).
  std::array<int, HOURS_IN_YEAR> Month{};
};

} // namespace openstudio::isomodel
