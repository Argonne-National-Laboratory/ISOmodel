/**********************************************************************
 * Refactored Header
 * - Preserves exact public/protected interface for binary compatibility.
 * - Replaced heavy include "SolarRadiation.hpp" with forward declaration.
 **********************************************************************/
#ifndef ISOMODEL_EPW_DATA_HPP
#define ISOMODEL_EPW_DATA_HPP

#include "ISOModelAPI.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "TimeFrame.hpp"

namespace openstudio {
namespace isomodel {

const int DBT = 0;
const int DPT = 1;
const int RH = 2;
const int EGH = 3;
const int EB = 4;
const int ED = 5;
const int WSPD = 6;

// Forward declaration allows removing the include
class SolarRadiation;

class ISOMODEL_API EpwData
{
protected:
  // Signature kept exactly as original (pass-by-value) for compatibility
  void parseHeader(std::string line);
  void parseData(std::string line, int row);
  
  std::string m_location, m_stationid;
  int m_timezone;
  double m_latitude, m_longitude;
  std::vector<std::vector<double> > m_data;

public:
  EpwData(void);
  ~EpwData(void);

  // loads data from an array, each block_size
  // number of values are the values for a column
  // (e.g. dry bulb temp, etc.)
  void loadData(int block_size, double* data);
  void loadData(std::string);
  std::string toISOData();

  // Getters (Inline implementations preserved for ABI compatibility)
  std::string location() {
    return m_location;
  }

  std::string stationid() {
    return m_stationid;
  }

  int timezone() {
    return m_timezone;
  }

  double latitude() {
    return m_latitude;
  }

  double longitude() {
    return m_longitude;
  }

  std::vector<std::vector<double> > data() {
    return m_data;
  }

};

}
}

#endif