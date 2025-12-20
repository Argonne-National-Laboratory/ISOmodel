#include "EpwData.hpp"
#include "SolarRadiation.hpp" // Moved include here to implement toISOData
#include <cstdlib> // for atof replacement logic if needed
#include <algorithm>

namespace openstudio {
namespace isomodel {

EpwData::EpwData(void)
{
  m_data.resize(7);
  m_latitude = 0.0;
  m_longitude = 0.0;
  m_timezone = 0;
}

EpwData::~EpwData(void)
{
}

void EpwData::parseHeader(std::string line)
{
  std::stringstream linestream(line);
  std::string s;
  
  // Refactored for readability and safety (std::stod)
  // Indices: 1=City, 5=StationID, 6=Lat, 7=Lon, 8=TZ
  int i = 0;
  while(std::getline(linestream, s, ',')) {
    switch (i) {
    case 1:
      m_location = s;
      break;
    case 5:
      m_stationid = s;
      break;
    case 6:
      try { m_latitude = std::stod(s); } catch(...) { m_latitude = 0.0; }
      break;
    case 7:
      try { m_longitude = std::stod(s); } catch(...) { m_longitude = 0.0; }
      break;
    case 8:
      try { m_timezone = (int)std::stod(s); } catch(...) { m_timezone = 0; }
      break;
    default:
      break;
    }
    i++;
    if (i > 8) break; // Optimization: stop parsing after timezone
  }
}

void EpwData::parseData(std::string line, int row)
{
  std::stringstream linestream(line);
  std::string s;
  int col = 0;
  
  // EPW Data Columns: 6,7,8,13,14,15,21 map to m_data[0..6]
  int i = 0;
  while(std::getline(linestream, s, ',')) {
    bool isTarget = false;
    switch (i) {
      case 6: case 7: case 8:
      case 13: case 14: case 15:
      case 21:
        isTarget = true;
        break;
      default:
        break;
    }

    if (isTarget) {
      // Use stod for safer double conversion than atof
      if (col < 7) { // Safety check
          try {
            m_data[col][row] = std::stod(s);
          } catch(...) {
            m_data[col][row] = 0.0;
          }
          col++;
      }
    }
    i++;
    if (i > 21) break; // Optimization: stop after last relevant column
  }
}

std::string EpwData::toISOData()
{
  // Implementation kept similar but cleaned up construction
  TimeFrame frames;
  SolarRadiation pos(&frames, this);
  pos.Calculate();
  
  std::stringstream sstream;
  
  auto write_vector = [&](const char* name, const std::vector<double>& vec, int limit) {
      sstream << name << std::endl;
      for (int i = 0; i < limit; i++) {
        sstream << i << "," << vec[i] << std::endl;
      }
  };

  write_vector("mdbt", pos.monthlyDryBulbTemp(), 12);
  write_vector("mwind", pos.monthlyWindspeed(), 12);
  write_vector("mEgh", pos.monthlyGlobalHorizontalRadiation(), 12);

  sstream << "hdbt" << std::endl;
  for (int i = 0; i < 12; i++) {
    sstream << i;
    for (int h = 0; h < 24; h++) {
      sstream << "," << pos.hourlyDryBulbTemp()[i][h];
    }
    sstream << std::endl;
  }
  
  sstream << "hEgh" << std::endl;
  for (int i = 0; i < 12; i++) {
    sstream << i;
    for (int h = 0; h < 24; h++) {
      sstream << "," << pos.hourlyGlobalHorizontalRadiation()[i][h];
    }
    sstream << std::endl;
  }
  
  sstream << "solar" << std::endl;
  for (int i = 0; i < 12; i++) {
    sstream << i;
    for (int s = 0; s < NUM_SURFACES; s++) {
      sstream << "," << pos.monthlySolarRadiation()[i][s];
    }
    sstream << std::endl;
  }
  
  return sstream.str();
}

void EpwData::loadData(int block_size, double* data)
{
  m_latitude = data[0];
  m_longitude = data[1];
  m_timezone = (int) data[2];
  
  double* ptr = data + 3;
  for (int c = 0; c < 7; c++) {
    std::vector<double>& col = m_data[c];
    col.resize(8760);
    for (int i = 0; i < block_size && i < 8760; ++i) {
      col[i] = *ptr; 
      ++ptr;
    }
  }
}

void EpwData::loadData(std::string fn)
{
  std::ifstream myfile(fn.c_str());
  
  // Optimization: Pre-allocate vectors
  for (int c = 0; c < 7; c++) {
    m_data[c].resize(8760);
  }

  if (myfile.is_open()) {
    std::string line;
    int lineCount = 0;
    int row = 0;
    
    // getline returns reference to stream, evaluates to true if good
    while (std::getline(myfile, line) && row < 8760) {
      lineCount++;
      if (lineCount == 1) {
        parseHeader(line);
      } else if (lineCount > 8) {
        parseData(line, row++);
      }
    }
    myfile.close();
  }
}

} // namespace isomodel
} // namespace openstudio