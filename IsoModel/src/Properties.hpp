
#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <fstream>

#include <iostream>
#include <map>
#include <string>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

namespace openstudio {

namespace isomodel {

/**
 * Unary function used in a transform_iterator that allows the map
 * iterator to return the keys
 */

struct KeyGetter: public std::unary_function<std::map<std::string, std::string>::value_type, std::string>
{
  std::string operator()(const std::map<std::string, std::string>::value_type& value) const;
};

/**
 * Map type object that contains key, value(string) properties. A Properties
 * instance can be constructed from a file. Each line is a property with the
 * key and value separated by =. For example,
 *
 * some.property = 3<br>
 * another.property = hello
 *
 * Key are case insensitive and stored in lower case.
 */
class Properties
{

private:
  std::map<std::string, std::string> map;

  void readFile(const std::string& file);

public:

  typedef boost::transform_iterator<KeyGetter, std::map<std::string, std::string>::const_iterator> key_iterator;

  /**
   * Creates an empty Properties.
   */
  Properties();

  /**
   * Creates a new Properties using the properties defined in the specified file.
   * Each line is a property with the key and value separated by =. For example,
   *
   * some.property = 3<br>
   * another.property = hello
   *
   * @param file the properties file path
   */
  Properties(const std::string& file);
  
  /**
  * Creates a new Properties using the properties defined in the specified
  * buildingFile and defaults from the specified defaultsFile
  */
  Properties(const std::string& buildingFile, const std::string& defaultFile);

  virtual ~Properties()
  {
  }

  /**
   * Puts a property into this Properties with
   * the specified key and value.
   *
   * @param key the property key
   * @param value the property value
   */
  void putProperty(const std::string& key, std::string value);

  /**
   * Puts a property into this Properties with
   * the specified key and value. Note that
   * even though the second argument can be passed
   * as a numeric value, it is stored as a string
   *
   * @param key the property key
   * @param value the property value
   */
  void putProperty(const std::string& key, double value);

  /**
   * Gets the property with the specified key.
   *
   * @param key the property key
   *
   * @return the value for that key, or boost::none
   * if the property is not found.
   */
  boost::optional<std::string> getProperty(const std::string& key) const;

  /**
   * Gets the property with the specified key as a double value
   *
   * @param key the property key
   *
   * @return the value for that key
   * @return boost::none if the specified property is not found or is not a double.
   */
  boost::optional<double> getPropertyAsDouble(const std::string& key) const;

  /**
   * Gets the property with the specified key as a vector of doubles. The
   * value is comma separated list of doubles that will be parsed into a
   * vector of doubles.
   *
   * @param key the property key
   *
   * @return the vector of doubles for that key
   * @return false if the specified property is not found or is not a double.
   */
  bool getPropertyAsDoubleVector(const std::string& key, std::vector<double>& vec) const;

  /**
   * Gets whether or not this Properties contains the specified key.
   *
   * @param key the property key
   */
  bool contains(const std::string& key) const;

  /**
   * Gets the start of an iterator over this Properties' keys.
   *
   * @return the start of an iterator over this Properties' keys.
   */
  key_iterator keys_begin() const
  {
    return key_iterator(map.begin());
  }

  /**
   * Gets the end of an iterator over this Properties' keys.
   *
   * @return the end of an iterator over this Properties' keys.
   */
  key_iterator keys_end() const
  {
    return key_iterator(map.end());
  }

  /**
   * Gets the number of properties in this Properties.
   *
   * @return the number of properties in this Properties.
   */
  int size() const
  {
    return map.size();
  }

};

}

}

#endif /* PROPERTIES_H_ */
