/// @file EpwData.cpp
/// @brief EnergyPlus Weather (EPW) file parser and hourly weather data container.
///
/// Parses .epw files to extract hourly dry-bulb temperature, wind speed,
/// global horizontal radiation, and other meteorological fields. Computes
/// monthly averages and diurnal profiles for use by the monthly and hourly
/// simulation models.
///
/// @author Brian Craig
/// @author Nick Collier
/// @author Brendan Albano
/// @author Ralph Muehleisen
/// @date 2013-11-05
/// @copyright Copyright Argonne National Laboratory
#include "EpwData.hpp"

#include "Constants.hpp"
#include "MathHelpers.hpp"
#include "SolarRadiation.hpp"
#include "TimeFrame.hpp"
#include "WeatherData.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib> // Added for std::strtod
#include <fstream>
#include <iostream>
#include <sstream>

namespace {
// Determine number of columns based on last enum value.  EpwDataCol is defined in EpwData.hpp
constexpr int kNumEpwDataCols =
    openstudio::isomodel::toIndex(openstudio::isomodel::EpwDataCol::WSPD) + 1;

//// --- EPW Data Indices (for internal storage vectors) ---
// EPW Columns of interest (0-based index in m_data):
// DBT = Dry Bulb Temp [C]  = col 6
// DPT = Dew Point Temp  [C] = col 7,
// RH = Relative Humidity [%] = col 8
// EGH = Global Horizontal Irradiance [W/m2] = col 13
// EB =  Beam Normal Irradiance [W/m2]=  col 14
// ED = Diffuse Horizontal Irradiance [W/m2] = col 15
// WSPD = Wind Speed Magnitude [m/s]= col 21

constexpr int EPW_FILE_COL_DBT = 6;
constexpr int EPW_FILE_COL_DPT = 7;
constexpr int EPW_FILE_COL_RH = 8;
constexpr int EPW_FILE_COL_EGH = 13;
constexpr int EPW_FILE_COL_EB = 14;
constexpr int EPW_FILE_COL_ED = 15;
constexpr int EPW_FILE_COL_WSPD = 21;

} // namespace


namespace openstudio::isomodel {

EpwData::EpwData() {
  // Pre-allocate the 7 data columns
  // m_data.resize(7);
  m_data.resize(kNumEpwDataCols); // size based on last enum value
  for (auto &col : m_data) {
    col.reserve(HOURS_IN_YEAR); // Optional optimization
  }
}

// Destructor defaulted in header

// Direct population of WeatherData to avoid string serialization
void EpwData::populateWeatherData(std::shared_ptr<WeatherData> wd) {
  if (!wd)
    return;

  TimeFrame frames;
  SolarRadiation pos(&frames, this);
  pos.Calculate();

  // Direct transfer for Vectors (std::vector<double> is compatible with Vector)
  wd->setMdbt(pos.monthlyDryBulbTemp());
  wd->setMwind(pos.monthlyWindspeed());
  wd->setMEgh(pos.monthlyGlobalHorizontalRadiation());

  // Convert and set matrices using helper from MathHelpers.hpp
  // Hourly data is 12 months x 24 hours
  wd->setMhdbt(toMatrix(pos.hourlyDryBulbTemp(), MONTHS_IN_YEAR, HOURS_IN_DAY));
  wd->setMhEgh(toMatrix(pos.hourlyGlobalHorizontalRadiation(), MONTHS_IN_YEAR, HOURS_IN_DAY));

  // Solar radiation is 12 months x 8 surfaces
  wd->setMsolar(toMatrix(pos.monthlySolarRadiation(), MONTHS_IN_YEAR, NUM_VERTICAL_SURFACES));
}

void EpwData::parseHeader(const std::string &line) {
  std::stringstream linestream(line);
  std::string segment;
  int i = 0;

  while (std::getline(linestream, segment, ',')) {
    switch (i) {
    case 1:
      m_location = segment;
      break;
    case 5:
      m_stationid = segment;
      break;
    case 6:
      try {
        m_latitude = std::stod(segment);
      } catch (...) {
        m_latitude = 0.0;
      }
      break;
    case 7:
      try {
        m_longitude = std::stod(segment);
      } catch (...) {
        m_longitude = 0.0;
      }
      break;
    case 8:
      try {
        m_timezone = std::stoi(segment);
      } catch (...) {
        m_timezone = 0;
      }
      break;
    }
    i++;
    if (i > 8)
      break; // Optimization: Stop after timezone
  }
}

void EpwData::parseData(const std::string &line, int row) {
  // Use pointer arithmetic and strtod directly on the buffer
  // to avoid creating substring allocations for every field.
  const char *pLine = line.c_str();
  size_t start = 0;
  size_t end = 0;
  int colIdx = 0;


  while ((end = line.find(',', start)) != std::string::npos) {
    int dataIndex = -1;

    switch (colIdx) {
    case EPW_FILE_COL_DBT:
      dataIndex = toIndex(EpwDataCol::DBT); // update to more modern access style
      break;
    case EPW_FILE_COL_DPT:
      dataIndex = toIndex(EpwDataCol::DPT);
      break;
    case EPW_FILE_COL_RH:
      dataIndex = toIndex(EpwDataCol::RH);
      break;
    case EPW_FILE_COL_EGH:
      dataIndex = toIndex(EpwDataCol::EGH);
      break;
    case EPW_FILE_COL_EB:
      dataIndex = toIndex(EpwDataCol::EB);
      break;
    case EPW_FILE_COL_ED:
      dataIndex = toIndex(EpwDataCol::ED);
      break;
    case EPW_FILE_COL_WSPD:
      dataIndex = toIndex(EpwDataCol::WSPD);
      break;
    }

    if (dataIndex != -1) {
      // strtod parses a double from the start pointer and updates pEnd
      // to point to the character after the number (usually the comma).
      char *pEnd = nullptr;
      double val = std::strtod(pLine + start, &pEnd);

      // If pEnd moved, a valid conversion occurred.
      // If pEnd == start, no conversion happened (e.g., empty string or invalid
      // char).
      if (pEnd != pLine + start) {
        m_data[dataIndex][row] = val;
      } else {
        m_data[dataIndex][row] = 0.0;
      }
    }

    start = end + 1;
    colIdx++;
    if (colIdx > EPW_FILE_COL_WSPD)
      break; // Stop after wind speed
  }
}

std::string EpwData::toISOData() {
  TimeFrame frames;
  SolarRadiation pos(&frames, this);
  pos.Calculate();

  std::stringstream sstream;

  auto write_csv = [&](const char *name, const std::vector<double> &vec, int limit) {
    sstream << name << "\n";
    for (int i = 0; i < limit; ++i) {
      sstream << i << "," << vec[i] << "\n";
    }
  };

  write_csv("mdbt", pos.monthlyDryBulbTemp(), MONTHS_IN_YEAR);
  write_csv("mwind", pos.monthlyWindspeed(), MONTHS_IN_YEAR);
  write_csv("mEgh", pos.monthlyGlobalHorizontalRadiation(), MONTHS_IN_YEAR);

  sstream << "hdbt\n";
  const auto &hdbt = pos.hourlyDryBulbTemp();
  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    sstream << i;
    for (int h = 0; h < HOURS_IN_DAY; ++h)
      sstream << "," << hdbt[i][h];
    sstream << "\n";
  }

  sstream << "hEgh\n";
  const auto &hegh = pos.hourlyGlobalHorizontalRadiation();
  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    sstream << i;
    for (int h = 0; h < HOURS_IN_DAY; ++h)
      sstream << "," << hegh[i][h];
    sstream << "\n";
  }

  sstream << "solar\n";
  const auto &msolar = pos.monthlySolarRadiation();
  for (int i = 0; i < MONTHS_IN_YEAR; ++i) {
    sstream << i;
    for (int s = 0; s < NUM_VERTICAL_SURFACES; ++s)
      sstream << "," << msolar[i][s];
    sstream << "\n";
  }

  return sstream.str();
}

void EpwData::loadData(int block_size, double *data) {
  if (!data)
    return;

  m_latitude = data[0];
  m_longitude = data[1];
  m_timezone = static_cast<int>(data[2]);

  double *ptr = data + 3;
  size_t rows = std::min(static_cast<size_t>(block_size), static_cast<size_t>(HOURS_IN_YEAR));

  // for (int c = 0; c < 7; ++c)  remove hardcoded column numbers
  for (int c = 0; c < kNumEpwDataCols; ++c) {
    m_data[c].resize(HOURS_IN_YEAR); // Ensure size
    for (size_t i = 0; i < rows; ++i) {
      m_data[c][i] = *ptr++;
    }
  }
}

void EpwData::loadData(const std::string &fn) {
  std::ifstream myfile(fn); // c_str() not needed in modern C++

  // Ensure vectors are ready
  for (auto &col : m_data) {
    col.assign(HOURS_IN_YEAR, 0.0);
  }

  if (myfile.is_open()) {
    std::string line;
    // Pre-allocate string memory for slight perf boost
    line.reserve(256);

    int lineCount = 0;
    int row = 0;

    while (std::getline(myfile, line) && row < HOURS_IN_YEAR) {
      lineCount++;
      if (lineCount == 1) {
        parseHeader(line);
      } else if (lineCount > 8) {
        parseData(line, row);
        row++;
      }
    }
    myfile.close();
  } else {
    std::cerr << "Failed to open EPW file: " << fn << std::endl;
  }
}

} // namespace openstudio::isomodel