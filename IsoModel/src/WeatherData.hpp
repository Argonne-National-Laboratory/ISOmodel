#pragma once
#include "ISOModelAPI.hpp"
#include "MathHelpers.hpp"

#include <memory>

namespace openstudio::isomodel {

class ISOMODEL_API WeatherData {
public:
  // Use compiler-generated default constructor/destructor
  WeatherData() = default;
  ~WeatherData() = default;

  /**
   * mean monthly Global Horizontal Radiation (W/m2)
   */
  Vector mEgh() const { return m_mEgh; }
  const Vector &mEghRef() const { return m_mEgh; }
  void setMEgh(const Vector &val) { m_mEgh = val; }

  /**
   * mean monthly dry bulb temp (C)
   */
  Vector mdbt() const { return m_mdbt; }
  const Vector &mdbtRef() const { return m_mdbt; }
  void setMdbt(const Vector &val) { m_mdbt = val; }

  /**
   * mean monthly wind speed; (m/s)
   */
  Vector mwind() const { return m_mwind; }
  const Vector &mwindRef() const { return m_mwind; }
  void setMwind(const Vector &val) { m_mwind = val; }

  /**
   * mean monthly total solar radiation (W/m2) on a vertical surface for each of
   * the 8 cardinal directions
   */
  Matrix msolar() const { return m_msolar; }
  const Matrix &msolarRef() const { return m_msolar; }
  void setMsolar(const Matrix &val) { m_msolar = val; }

  /**
   * mean monthly dry bulb temp for each of the 24 hours of the day (C)
   */
  Matrix mhdbt() const { return m_mhdbt; }
  const Matrix &mhdbtRef() const { return m_mhdbt; }
  void setMhdbt(const Matrix &val) { m_mhdbt = val; }

  /**
   * mean monthly Global Horizontal Radiation for each of the 24 hours of the
   * day (W/m2)
   */
  Matrix mhEgh() const { return m_mhEgh; }
  const Matrix &mhEghRef() const { return m_mhEgh; }
  void setMhEgh(const Matrix &val) { m_mhEgh = val; }

private:
  Matrix m_msolar;
  Matrix m_mhdbt;
  Matrix m_mhEgh;
  Vector m_mEgh;
  Vector m_mdbt;
  Vector m_mwind;
};

} // namespace openstudio::isomodel
