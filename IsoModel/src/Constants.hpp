#ifndef ISOMODEL_CONSTANTS_HPP
#define ISOMODEL_CONSTANTS_HPP

namespace openstudio {
    namespace isomodel {

        // --- Time Constants ---

        constexpr int MONTHS_IN_YEAR = 12;
        constexpr int HOURS_IN_DAY = 24;
        constexpr int HOURS_IN_WEEK = 168;
        constexpr int DAYS_IN_WEEK = 7;
        constexpr int DAYS_IN_YEAR = 365;
        //constexpr int HOURS_IN_YEAR = 8760;
        constexpr int HOURS_IN_YEAR = DAYS_IN_YEAR * HOURS_IN_DAY;
        constexpr int SECONDS_IN_HOUR = 3600;

        // --- Geometry & Directions ---
        // 8 Compass directions (N, NE, E, SE, S, SW, W, NW)
        constexpr int NUM_COMPASS_DIRECTIONS = 8;
        // 8 Compass directions + 1 Roof/Horizontal = 9 (Used often in loops)
        constexpr int NUM_TOTAL_SURFACES = 9;

        //// --- Math & Physics ---
        //constexpr double PI = 3.14159265358979323846;
        constexpr double SMALL_EPSILON = 1e-15;  // Used for safe division/avoiding zero

        //// --- Unit Conversions ---
        constexpr double KWH_TO_MJ = 3.6;              // 1 kWh = 3.6 MJ
        constexpr double MJ_TO_KWH = 1.0 / 3.6;
        constexpr double MJ_TO_WATT_HOURS = 277.777778; // 1 MJ = 277.78 Wh
        constexpr double W_TO_KW = 0.001;

        //// --- EPW Data Indices (for internal storage vectors) ---
        //enum EpwIndex {
        //    IDX_DBT = 0, // Dry Bulb Temp
        //    IDX_DPT = 1, // Dew Point Temp
        //    IDX_RH = 2, // Relative Humidity
        //    IDX_EGH = 3, // Global Horizontal Radiation
        //    IDX_EB = 4, // Direct Normal/Beam Radiation
        //    IDX_ED = 5, // Diffuse Radiation
        //    IDX_WSPD = 6  // Wind Speed
        //};

    } // namespace isomodel
} // namespace openstudio

#endif // ISOMODEL_CONSTANTS_HPP#pragma once
