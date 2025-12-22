/**********************************************************************
 * Copyright (c) 2008-2013, Alliance for Sustainable Energy.
 * All rights reserved.
 **********************************************************************/
#ifndef ISOMODEL_EPW_DATA_HPP
#define ISOMODEL_EPW_DATA_HPP

#include "ISOModelAPI.hpp"

#include <string>
#include <vector>
#include <memory>

// Forward declaration
namespace openstudio::isomodel {
    class SolarRadiation;
}

namespace openstudio::isomodel {

    // Column constants
    constexpr int DBT = 0;
    constexpr int DPT = 1;
    constexpr int RH = 2;
    constexpr int EGH = 3;
    constexpr int EB = 4;
    constexpr int ED = 5;
    constexpr int WSPD = 6;

    class ISOMODEL_API EpwData
    {
    protected:
        // Internal helpers - implementation details can change as long as signature matches
        void parseHeader(std::string line);
        void parseData(std::string line, int row);

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
        void loadData(int block_size, double* data);
        void loadData(std::string fn);
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
    };

} // namespace openstudio::isomodel

#endif // ISOMODEL_EPW_DATA_HPP