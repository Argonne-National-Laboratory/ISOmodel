#include "EpwData.hpp"

namespace openstudio {
namespace isomodel {

EpwData::EpwData(void)
{
  m_data.resize(7);
}

EpwData::~EpwData(void)
{
}

void EpwData::parseHeader(std::string line)
{
  std::stringstream linestream(line);
  std::string s;
  //cout << "Weather Location Header: "<<endl;
  for (int i = 0; i < 10; i++) {
    std::getline(linestream, s, ',');
    switch (i) {
    case 1:
      m_location = s;
      //cout << "\tLocation: " << s <<endl;
      break;
    case 5:
      m_stationid = s;
      //cout << "\tStation ID: " << s <<endl;
      break;
    case 6:
      m_latitude = atof(s.c_str());
      //cout << "\tLatitude: " << s <<endl;
      break;
    case 7:
      m_longitude = atof(s.c_str());
      //cout << "\tLongitude: " << s <<endl;
      break;
    case 8:
      m_timezone = (int) atof(s.c_str());
      //cout << "\tTimezone: " << s <<endl;
      break;
    default:
      break;
    }
  }
}
void EpwData::parseData(std::string line, int row)
{
  std::stringstream linestream(line);
  std::string s;
  int col = 0;
  for (int i = 0; i < 22; i++) {
    std::getline(linestream, s, ',');
    switch (i) {
    case 6:
    case 7:
    case 8:
    case 13:
    case 14:
    case 15:
    case 21:
      m_data[col++][row] = (double) ::atof(s.c_str());
      break;
    default:
      break;
    }
  }
}
std::string EpwData::toISOData()
{
  std::string results;
  TimeFrame frames;

  SolarRadiation pos(&frames, this);
  pos.Calculate();
  std::stringstream sstream;
  sstream << "mdbt" << std::endl;
  for (int i = 0; i < 12; i++) {
    sstream << i << "," << pos.monthlyDryBulbTemp()[i] << std::endl;
  }
  sstream << "mwind" << std::endl;
  for (int i = 0; i < 12; i++) {
    sstream << i << "," << pos.monthlyWindspeed()[i] << std::endl;
  }
  sstream << "mEgh" << std::endl;
  for (int i = 0; i < 12; i++) {
    sstream << i << "," << pos.monthlyGlobalHorizontalRadiation()[i] << std::endl;
  }
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
  // first 3 doubles are latitude, longitude, tz
  m_latitude = data[0];
  m_longitude = data[1];
  m_timezone = (int) data[2];
  // each block_size number of doubles is a column of data
  double* ptr = data + 3;
  for (int c = 0; c < 7; c++) {
    std::vector<double>& col = m_data[c];
    col.resize(8760);
    for (int i = 0; i < block_size; ++i) {
      col[i] = *ptr; //data[(c * block_size) + i];
      ++ptr;
    }
  }
}

void EpwData::loadData(std::string fn)
{
  std::string line;
  std::ifstream myfile(fn.c_str());
  int i = 0;
  int row = 0;

  for (int c = 0; c < 7; c++) {
    m_data[c].resize(8760);
  }
  if (myfile.is_open()) {
    while (myfile.good() && row < 8760) {
      i++;
      getline(myfile, line);
      if (i == 1) {
        parseHeader(line);
      } else if (i > 8) {
        parseData(line, row++);
      }
    }
    myfile.close();
  }
}
}
}
