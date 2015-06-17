#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/tokenizer.hpp>

#include "Properties.hpp"

using namespace std;

namespace openstudio {

namespace isomodel {

string KeyGetter::operator()(const map<string, string>::value_type& value) const
{
  return value.first;
}

void str_trim(string &str)
{
  size_t pos = str.find_last_not_of(" \t\r");
  if (pos != string::npos) {
    str.erase(pos + 1);
    pos = str.find_first_not_of(" \t");
    if (pos != string::npos) {
      str.erase(0, pos);
    }
  } else {
    str = "";
  }
}

Properties::Properties()
{
}

Properties::Properties(const std::string& file) {
  readFile(file);
}

Properties::Properties(const std::string& buildingFile, const std::string& defaultsFile) {
  // Read the buildingsFile first, then the defaultsFile because map::insert does not overwrite
  // values and if a property exists in both files, we want the buildingFile to take precedence
  // over the defaultsFile.
  readFile(buildingFile);
  readFile(defaultsFile);
}

bool Properties::getPropertyAsDoubleVector(const std::string& key, std::vector<double>& vec) const {
  if (auto val = getProperty(key)) {
    vec.clear();
    // tokenize the line using boost's escaped list separator which parses CSV format
    boost::tokenizer<boost::escaped_list_separator<char> > tok(*val);
    // assign those values to the vector
    try {
      for (auto& item : tok) {
        vec.push_back(std::stod(item));
      }
    }
    catch (std::invalid_argument& ex) {
      return false; // Cannot be converted to a double.
    }
    return true;
  }
  else {
    return false; // Key missing.
  }
}

bool Properties::getPropertyAsDoubleVector(const std::string& key, Vector& vec) const {
  if (auto val = getProperty(key)) {
    vec.clear();
    // tokenize the line using boost's escaped list separator which parses CSV format
    boost::tokenizer<boost::escaped_list_separator<char> > tok(*val);
    // assign those values to the vector

    // Make sure the ublas vector is the same size as the number of tokens.
    auto numTokens = std::distance(std::begin(tok), std::end(tok));
    if (vec.size() != numTokens) {
      vec.resize(numTokens);
    }

    try {
      auto index = 0;
      for (auto& item : tok) {
        vec[index] = (std::stod(item));
        ++index;
      }
    }
    catch (std::invalid_argument& ex) {
      return false; // Cannot be converted to a double.
    }
    return true;
  }
  else {
    return false; // Key missing.
  }
}

boost::optional<double> Properties::getPropertyAsDouble(const std::string& key) const
{
  if (auto val = getProperty(key)) {
    try {
      return std::stod(*val);
    } catch (std::invalid_argument& ex) {
      return boost::none; // Cannot be converted to a double.
    }
  }
  else {
    return boost::none; // Key missing.
  }
}


void Properties::putProperty(const string& key, string value)
{
  std::string k(key);
  std::transform(key.begin(), key.end(), k.begin(), ::tolower);
  map[k] = value;
}

void Properties::putProperty(const string& key, double value)
{
  std::string k(key);
  std::transform(key.begin(), key.end(), k.begin(), ::tolower);
  map[k] = std::to_string(value);
}

bool Properties::contains(const string& key) const
{

  std::string k(key);
  std::transform(key.begin(), key.end(), k.begin(), ::tolower);
  return map.find(k) != map.end();
}

boost::optional<string> Properties::getProperty(const string& key) const
{
  std::string k(key);
  std::transform(key.begin(), key.end(), k.begin(), ::tolower);
  std::map<string, string>::const_iterator iter = map.find(k);
  if (iter == map.end())
    return boost::none;
  else
    return iter->second;
}

void Properties::readFile(const std::string& file)
{
  ifstream in_file(file.c_str(), ios_base::in);
  if (in_file.is_open()) {
    std::string line;
    int line_num = 0;
    while (std::getline(in_file, line)) {
      ++line_num;
      str_trim(line);
      if (line.length() > 0 && line[0] != '#') {

        size_t pos = line.find_first_of("=");
        if (pos == string::npos)
          throw new domain_error("Invalid format in file '" + file + "' on line " + std::to_string(line_num));
        string key = line.substr(0, pos);
        str_trim(key);
        if (key.length() == 0)
          throw new domain_error("Missing property key in properties file '" + file + "'on line " + std::to_string(line_num));
        string value = "";
        if (line.length() > pos) {
          // this makes sure we only try to get value if it exists
          value = line.substr(pos + 1, line.length());
        }
        str_trim(value);
        if (value.length() == 0)
          throw new domain_error("Missing property value in properties file '" + file + "'on line " + std::to_string(line_num));

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        map.insert(std::make_pair(key, value));
      }
    }
    in_file.close();
  } else {
    throw new domain_error("Error opening properties file '" + file + "'");
  }
}

}

}
