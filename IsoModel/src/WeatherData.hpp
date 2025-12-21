/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/
#ifndef ISOMODEL_WEATHER_DATA_HPP
#define ISOMODEL_WEATHER_DATA_HPP

#include "ISOModelAPI.hpp"

#ifdef ISOMODEL_STANDALONE
#include "Vector.hpp"
#include "Matrix.hpp"
#else
#include "../utilities/data/Vector.hpp"
#include "../utilities/data/Matrix.hpp"
#endif

#include <memory>

namespace openstudio::isomodel {

class ISOMODEL_API WeatherData
{
public:
  // Use compiler-generated default constructor/destructor
  WeatherData() = default;
  ~WeatherData() = default;

  /**
   * mean monthly Global Horizontal Radiation (W/m2)
   */
  Vector mEgh() { return m_mEgh; }
  void setMEgh(const Vector& val) { m_mEgh = val; }

  /**
   * mean monthly dry bulb temp (C)
   */
  Vector mdbt() { return m_mdbt; }
  void setMdbt(const Vector& val) { m_mdbt = val; }

  /**
   * mean monthly wind speed; (m/s) 
   */
  Vector mwind() { return m_mwind; }
  void setMwind(const Vector& val) { m_mwind = val; }

  /**
   * mean monthly total solar radiation (W/m2) on a vertical surface for each of the 8 cardinal directions
   */
  Matrix msolar() { return m_msolar; }
  void setMsolar(const Matrix& val) { m_msolar = val; }

  /**
   * mean monthly dry bulb temp for each of the 24 hours of the day (C)
   */
  Matrix mhdbt() { return m_mhdbt; }
  void setMhdbt(const Matrix& val) { m_mhdbt = val; }

  /**
   * mean monthly Global Horizontal Radiation for each of the 24 hours of the day (W/m2)
   */
  Matrix mhEgh() { return m_mhEgh; }
  void setMhEgh(const Matrix& val) { m_mhEgh = val; }

private:
  Matrix m_msolar;
  Matrix m_mhdbt;
  Matrix m_mhEgh;
  Vector m_mEgh;
  Vector m_mdbt;
  Vector m_mwind;
};

} // namespace openstudio::isomodel
#endif // ISOMODEL_WEATHER_DATA_HPP