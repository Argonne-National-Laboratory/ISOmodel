#include "EpwData.hpp"
#include "SolarRadiation.hpp" 
#include "TimeFrame.hpp" 
#include "Constants.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cmath>

namespace openstudio::isomodel {

    EpwData::EpwData()
    {
        // Pre-allocate the 7 data columns
        m_data.resize(7);
        for (auto& col : m_data) {
            col.reserve(hoursInYear); // Optional optimization
        }
    }

    // Destructor defaulted in header

    void EpwData::parseHeader(std::string line)
    {
        std::stringstream linestream(line);
        std::string segment;
        int i = 0;

        while (std::getline(linestream, segment, ',')) {
            switch (i) {
            case 1: m_location = segment; break;
            case 5: m_stationid = segment; break;
            case 6: 
                try { m_latitude = std::stod(segment); } 
                catch (...) { m_latitude = 0.0; } 
                break;
            case 7: 
                try { m_longitude = std::stod(segment); } 
                catch (...) { m_longitude = 0.0; } 
                break;
            case 8: 
                try { m_timezone = std::stoi(segment); } 
                catch (...) { m_timezone = 0; } 
                break;
            }
            i++;
            if (i > 8) break; // Optimization: Stop after timezone
        }
    }

    void EpwData::parseData(std::string line, int row)
    {
        // Optimized manual parsing loop avoids stringstream overhead
        size_t start = 0;
        size_t end = 0;
        int colIdx = 0;
        int targetCol = 0;

        // EPW Columns of interest (0-based index in m_data):
        // 6->0 (DBT), 7->1 (DPT), 8->2 (RH), 13->3 (EGH), 14->4 (EB), 15->5 (ED), 21->6 (WSPD)
        
        while ((end = line.find(',', start)) != std::string::npos) {
            bool isTarget = false;
            int dataIndex = -1;

            switch (colIdx) {
                case 6:  dataIndex = 0; isTarget = true; break;
                case 7:  dataIndex = 1; isTarget = true; break;
                case 8:  dataIndex = 2; isTarget = true; break;
                case 13: dataIndex = 3; isTarget = true; break;
                case 14: dataIndex = 4; isTarget = true; break;
                case 15: dataIndex = 5; isTarget = true; break;
                case 21: dataIndex = 6; isTarget = true; break;
            }

            if (isTarget) {
                try {
                    // Fast substring view would be better in C++17, but std::string substr is safe
                    // Note: std::stod can throw on empty/invalid strings
                    std::string val = line.substr(start, end - start);
                    if (!val.empty()) {
                        m_data[dataIndex][row] = std::stod(val);
                    } else {
                        m_data[dataIndex][row] = 0.0;
                    }
                } catch (...) {
                    m_data[dataIndex][row] = 0.0;
                }
            }

            start = end + 1;
            colIdx++;
            if (colIdx > 21) break; // Stop after wind speed
        }
        
        // Handle last column if needed (though WSPD is at 21, usually not last)
    }

    std::string EpwData::toISOData()
    {
        TimeFrame frames;
        SolarRadiation pos(&frames, this);
        pos.Calculate();

        std::stringstream sstream;

        auto write_csv = [&](const char* name, const std::vector<double>& vec, int limit) {
            sstream << name << "\n";
            for (int i = 0; i < limit; ++i) {
                sstream << i << "," << vec[i] << "\n";
            }
        };

        write_csv("mdbt", pos.monthlyDryBulbTemp(), monthsInYear);
        write_csv("mwind", pos.monthlyWindspeed(), monthsInYear);
        write_csv("mEgh", pos.monthlyGlobalHorizontalRadiation(), monthsInYear);

        sstream << "hdbt\n";
        const auto& hdbt = pos.hourlyDryBulbTemp();
        for (int i = 0; i < monthsInYear; ++i) {
            sstream << i;
            for (int h = 0; h < hoursInDay; ++h) sstream << "," << hdbt[i][h];
            sstream << "\n";
        }

        sstream << "hEgh\n";
        const auto& hegh = pos.hourlyGlobalHorizontalRadiation();
        for (int i = 0; i < monthsInYear; ++i) {
            sstream << i;
            for (int h = 0; h < hoursInDay; ++h) sstream << "," << hegh[i][h];
            sstream << "\n";
        }

        sstream << "solar\n";
        const auto& msolar = pos.monthlySolarRadiation();
        for (int i = 0; i < monthsInYear; ++i) {
            sstream << i;
            for (int s = 0; s < 8; ++s) sstream << "," << msolar[i][s]; // NUM_SURFACES = 8
            sstream << "\n";
        }

        return sstream.str();
    }

    void EpwData::loadData(int block_size, double* data)
    {
        if (!data) return;

        m_latitude = data[0];
        m_longitude = data[1];
        m_timezone = static_cast<int>(data[2]);

        double* ptr = data + 3;
        size_t rows = std::min(static_cast<size_t>(block_size), static_cast<size_t>(hoursInYear));

        for (int c = 0; c < 7; ++c) {
            m_data[c].resize(hoursInYear); // Ensure size
            for (size_t i = 0; i < rows; ++i) {
                m_data[c][i] = *ptr++;
            }
        }
    }

    void EpwData::loadData(std::string fn)
    {
        std::ifstream myfile(fn); // c_str() not needed in modern C++

        // Ensure vectors are ready
        for (auto& col : m_data) {
            col.assign(hoursInYear, 0.0);
        }

        if (myfile.is_open()) {
            std::string line;
            // Pre-allocate string memory for slight perf boost
            line.reserve(256); 

            int lineCount = 0;
            int row = 0;

            while (std::getline(myfile, line) && row < hoursInYear) {
                lineCount++;
                if (lineCount == 1) {
                    parseHeader(line);
                }
                else if (lineCount > 8) {
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