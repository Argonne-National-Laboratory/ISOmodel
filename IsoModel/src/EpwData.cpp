#include "EpwData.hpp"
#include <vector>
#include <iostream>

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
      //std::string s;
      char * cstr = new char[line.length() + 1];
      strcpy(cstr, line.c_str());
      char * word; //34 commas
      int col = 0;
      for(int i = 0; i < 22; i++){
            if(i == 0){
                  word = strtok(cstr, ",");
            }
            else{
                  word = strtok(NULL, ",");
            }
            switch(i){
                  case 6:
                  case 7:
                  case 8:
                  case 13:
                  case 14:
                  case 15:
                  case 21:
                  {
                        double x = strtod(word, NULL);
                        m_data[col++][row] = x;
                        //m_data[col++][row] = (double) (*word);
                        //std::string s(word);
                        //m_data[col++][row] = (double) ::atof(s.c_str());

                  }
                        break;
                  default:
                        break;
            }
      }
      //free(cstr);
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
      int i = 0;
      int row = 0;
      std::string line = "";
      char * buffer;
      std::vector <std::string> data;

      FILE * inputfile = fopen(fn.c_str(), "r");

      fseek(inputfile, 0, SEEK_END);
      int buffersize = ftell(inputfile);
      rewind(inputfile);

      buffer = new char[buffersize];
      size_t readcounter = fread(buffer, sizeof(char), buffersize, inputfile);
      int count = 0;
      char * memcounter = buffer;
      while(count < readcounter){
            line.push_back(*memcounter);
            if(*memcounter == 13 || *memcounter == 10 || *memcounter == '\n'){
                  data.push_back(line);
                  line.clear();
            }
            *memcounter++; count++;
      }
      for (int c = 0; c < 7; c++){
            m_data[c].resize(8760);
      }
      while(row < 8760 && i < data.size()){
            line = data[i];
            i++;
            if(i == 1){
                  parseHeader(line);
            }
            else if(i > 8){
                  parseData(line, row++);
            }
      }
      free(buffer);
      fclose(inputfile);
}
}
}
