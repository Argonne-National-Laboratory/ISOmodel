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
#ifndef ISOMODEL_POPULATION_HPP
#define ISOMODEL_POPULATION_HPP

#include <string>

namespace openstudio {
namespace isomodel {
class Population
{
public:
  Population(void);
  ~Population(void);

  /**
  * First occupied hour (0-23). Note that hoursStart() and hoursEnd() form a closed interval.
  * For example, a "nine to five" eight hour day would have hoursStart() == 9 and hoursEnd() == 16.
  */
  double hoursStart() const {
    return m_hoursStart;
  }

  void setHoursStart(double value) {
    m_hoursStart = value;
  }

  /**
  * Last occupied hour (0-23). Note that hoursStart() and hoursEnd() form a closed interval.
  * For example, a "nine to five" eight hour day would have hoursStart() == 9 and hoursEnd() == 16.
  */
  double hoursEnd() const {
    return m_hoursEnd;
  }

  void setHoursEnd(double value) {
    m_hoursEnd = value;
  }

  /**
  * First occupied Day (0-6). Note that daysStart() and daysEnd() form a closed interval.
  * For example, a "mondey to friday" five day work week would have daysStart() == 1 and
  * daysEnd() == 5.
  */
  double daysStart() const {
    return m_daysStart;
  }

  void setDaysStart(double value) {
    m_daysStart = value;
  }

  /**
  * Last occupied Day (0-6). Note that daysStart() and daysEnd() form a closed interval.
  * For example, a "mondey to friday" five day work week would have daysStart() == 1 and
  * daysEnd() == 5.
  */
  double daysEnd() const {
    return m_daysEnd;
  }

  void setDaysEnd(double value) {
    m_daysEnd = value;
  }

  /**
  * People density occupied (m2/person).
  */
  double densityOccupied() const {
    return m_densityOccupied;
  }

  void setDensityOccupied(double value) {
    m_densityOccupied = value;
  }

  /**
  * People density unoccupied (m2/person).
  */
  double densityUnoccupied() const {
    return m_densityUnoccupied;
  }

  void setDensityUnoccupied(double value) {
    m_densityUnoccupied = value;
  }

  /**
  * Heat gain per person (W/m2).
  */
  double heatGainPerPerson() const {
    return m_heatGainPerPerson;
  }

  void setHeatGainPerPerson(double value) {
    m_heatGainPerPerson = value;
  }

  // TODO: These properties aren't used by the simulations yet -BAA@2015-06-18
  /**
  *
  */
  std::string scheduleFilePath() const {
  return m_scheduleFilePath;
  }

  void setScheduleFilePath(std::string scheduleFilePath) {
  m_scheduleFilePath = scheduleFilePath;
  }

private:
  double m_hoursEnd;
  double m_hoursStart;
  double m_daysEnd;
  double m_daysStart;
  double m_densityOccupied;
  double m_densityUnoccupied;
  double m_heatGainPerPerson;

  // TODO: These properties aren't used by the simulations yet -BAA@2015-06-18
  std::string m_scheduleFilePath;
};

} // isomodel
} // openstudio
#endif // ISOMODEL_POPULATION_HPP
