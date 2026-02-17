#pragma once
#include "ISOModelAPI.hpp"

#include <string>

namespace openstudio::isomodel {

class ISOMODEL_API Population {
public:
  // Use compiler-generated default constructor/destructor
  Population() = default;
  ~Population() = default;

  /// First occupied hour (0-23). Note that hoursStart() and hoursEnd() form a
  /// closed interval. For example, a "nine to five" eight hour day would have
  /// hoursStart() == 9 and hoursEnd() == 16.
  [[nodiscard]] double hoursStart() const noexcept { return m_hoursStart; }
  void setHoursStart(double value) { m_hoursStart = value; }

  /// Last occupied hour (0-23). Note that hoursStart() and hoursEnd() form a
  /// closed interval. For example, a "nine to five" eight hour day would have
  /// hoursStart() == 9 and hoursEnd() == 16.
  [[nodiscard]] double hoursEnd() const noexcept { return m_hoursEnd; }
  void setHoursEnd(double value) { m_hoursEnd = value; }

  /// First occupied Day (0-6). Note that daysStart() and daysEnd() form a closed
  /// interval. For example, a "mondey to friday" five day work week would have
  /// daysStart() == 1 and daysEnd() == 5.
  [[nodiscard]] double daysStart() const noexcept { return m_daysStart; }
  void setDaysStart(double value) { m_daysStart = value; }

  /// Last occupied Day (0-6). Note that daysStart() and daysEnd() form a closed
  /// interval. For example, a "mondey to friday" five day work week would have
  /// daysStart() == 1 and daysEnd() == 5.
  [[nodiscard]] double daysEnd() const noexcept { return m_daysEnd; }
  void setDaysEnd(double value) { m_daysEnd = value; }

  /// People density occupied (m2/person).
  [[nodiscard]] double densityOccupied() const noexcept { return m_densityOccupied; }
  void setDensityOccupied(double value) { m_densityOccupied = value; }

  /// People density unoccupied (m2/person).
  [[nodiscard]] double densityUnoccupied() const noexcept { return m_densityUnoccupied; }
  void setDensityUnoccupied(double value) { m_densityUnoccupied = value; }

  /// Heat gain per person (W/m2).
  [[nodiscard]] double heatGainPerPerson() const noexcept { return m_heatGainPerPerson; }
  void setHeatGainPerPerson(double value) { m_heatGainPerPerson = value; }

  // TODO: These properties aren't used by the simulations yet -BAA@2015-06-18
  [[nodiscard]] std::string scheduleFilePath() const { return m_scheduleFilePath; }
  void setScheduleFilePath(std::string scheduleFilePath) { m_scheduleFilePath = scheduleFilePath; }

private:
  // In-class initialization prevents undefined behavior and potential
  // division-by-zero
  double m_hoursStart = 0.0;
  double m_hoursEnd = 24.0;
  double m_daysStart = 1.0;
  double m_daysEnd = 5.0;
  double m_densityOccupied = 1.0;
  double m_densityUnoccupied = 0.0001;
  double m_heatGainPerPerson = 100.0;

  // TODO: These properties aren't used by the simulations yet -BAA@2015-06-18
  std::string m_scheduleFilePath;
};

} // namespace openstudio::isomodel
