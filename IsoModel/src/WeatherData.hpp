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
#ifndef ISOMODEL_WEATHER_DATA_HPP
#define ISOMODEL_WEATHER_DATA_HPP

#include "ISOModelAPI.hpp"

#include "Vector.hpp"
#include "Matrix.hpp"

#include <memory>

namespace openstudio {
namespace isomodel {

class ISOMODEL_API WeatherData
{
public:
  WeatherData(void);
  ~WeatherData(void);

  /**
   * mean monthly Global Horizontal Radiation (W/m2)
   */
  Vector mEgh() {
    return m_mEgh;
  }

  void setMEgh(Vector val) {
    m_mEgh = val;
  }

  /**
   * mean monthly dry bulb temp (C)
   */
  Vector mdbt() {
    return m_mdbt;
  }

  void setMdbt(Vector val) {
    m_mdbt = val;
  }

  /**
   * mean monthly wind speed; (m/s) 
   */
  Vector mwind() {
    return m_mwind;
  }

  void setMwind(Vector val) {
    m_mwind = val;
  }


  /**
   * mean monthly total solar radiation (W/m2) on a vertical surface for each of the 8 cardinal directions
   */
  Matrix msolar() {
    return m_msolar;
  }

  void setMsolar(Matrix val) {
    m_msolar = val;
  }

  /**
   * mean monthly dry bulb temp for each of the 24 hours of the day (C)
   */
  Matrix mhdbt() {
    return m_mhdbt;
  }

  void setMhdbt(Matrix val) {
    m_mhdbt = val;
  }

  /**
   * mean monthly Global Horizontal Radiation for each of the 24 hours of the day (W/m2)
   */
  Matrix mhEgh() {
    return m_mhEgh;
  }

  void setMhEgh(Matrix val) {
    m_mhEgh = val;
  }

private:
  Matrix m_msolar;
  Matrix m_mhdbt;
  Matrix m_mhEgh;
  Vector m_mEgh;
  Vector m_mdbt;
  Vector m_mwind;
};
}
}

#endif
