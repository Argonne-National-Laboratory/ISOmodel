#pragma once
#include "ISOModelAPI.hpp"

#include <memory>
#include <string>
#include <vector>

// Forward declaration
namespace openstudio::isomodel {
class SolarRadiation;
}

namespace openstudio::isomodel {

// Column constants using enum class for better type safety
enum class EpwDataCol : int {
  DBT = 0,
  DPT = 1,
  RH = 2,
  EGH = 3,
  EB = 4,
  ED = 5,
  WSPD = 6,
};

constexpr int toIndex(EpwDataCol c) noexcept {
  return static_cast<int>(c);
}
constexpr int DBT = toIndex(EpwDataCol::DBT);
constexpr int DPT = toIndex(EpwDataCol::DPT);
constexpr int RH = toIndex(EpwDataCol::RH);
constexpr int EGH = toIndex(EpwDataCol::EGH);
constexpr int EB = toIndex(EpwDataCol::EB);
constexpr int ED = toIndex(EpwDataCol::ED);
constexpr int WSPD = toIndex(EpwDataCol::WSPD);

class ISOMODEL_API EpwData {
protected:
  // Internal helpers - implementation details can change as long as signature
  // matches
  void parseHeader(const std::string &line);
  void parseData(const std::string &line, int row);

  std::string m_location;
  std::string m_stationid;
  int m_timezone = 0;
  double m_latitude = 0.0;
  double m_longitude = 0.0;

  // In-class initialization for safety
  std::vector<std::vector<double>> m_data;

public:
  // Modernized: Defaulted constructor/destructor
  EpwData();
  ~EpwData() = default;

  // loads data from an array, each block_size
  // number of values are the values for a column
  // (e.g. dry bulb temp, etc.)
  void loadData(int block_size, double *data);
  void loadData(const std::string &fn);
  std::string toISOData();

  // Getters
  std::string location() const { return m_location; }
  std::string stationid() const { return m_stationid; }
  int timezone() const { return m_timezone; }
  double latitude() const { return m_latitude; }
  double longitude() const { return m_longitude; }

  // Note: Returning by value (copy) is the original interface.
  // ideally this would return const reference, but we must preserve ABI.
  std::vector<std::vector<double>> data() const { return m_data; }

  // Optimization: Return const reference to avoid copy
  const std::vector<std::vector<double>> &dataRef() const { return m_data; }

  // new structure for streaming weather data into WeatherData object
  void populateWeatherData(std::shared_ptr<class WeatherData> wd);
};

} // namespace openstudio::isomodel

