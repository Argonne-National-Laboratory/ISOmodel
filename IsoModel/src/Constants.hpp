#ifndef ISOMODEL_CONSTANTS_HPP
#define ISOMODEL_CONSTANTS_HPP

namespace openstudio {
    namespace isomodel {

        #ifndef DBL_MAX
        #define DBL_MAX    1.7976931348623157E+308
        #endif
        #ifndef DBL_MIN
        #define DBL_MIN    2.2250738585072014E-308
        #endif


        //// --- Math & Physics ---
        constexpr double PI = 3.14159265358979323846;
        constexpr double smallEpsilon = 1e-15;  // Used for safe division/avoiding zero




        // --- Time Constants ---

        constexpr int monthsInYear = 12;
        constexpr int hoursInDay = 24;
        constexpr int hoursInWeek = 168;
        constexpr int daysInWeek = 7;
        constexpr int daysInYear = 365;
        //constexpr int hoursInYear = 8760;
        constexpr int hoursInYear = daysInYear * hoursInDay;
        constexpr int secondsInHour = 3600;

        // Start hour for a standard weekday in EECALC
        constexpr int EECALC_WEEKDAY_START = 7;

        // Constants
        const double daysInMonth[] =
        { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        const double hoursInMonth[] =
        { 744, 672, 744, 720, 744, 720, 744, 744, 720, 744, 720, 744 };
        const double megasecondsInMonth[] =
        { 2.6784, 2.4192, 2.6784, 2.592, 2.6784, 2.592, 2.6784, 2.6784, 2.592, 2.6784, 2.592, 2.6784 };
        const double monthFractionOfYear[] =
        { 0.0849315068493151, 0.0767123287671233, 0.0849315068493151, 0.0821917808219178, 0.0849315068493151, 0.0821917808219178, 0.0849315068493151,
            0.0849315068493151, 0.0821917808219178, 0.0849315068493151, 0.0821917808219178, 0.0849315068493151 };


        // --- Geometry & Directions ---
        // 8 Compass directions (N, NE, E, SE, S, SW, W, NW)
        constexpr int numCompassDIrections = 8;
        // 8 Compass directions + 1 Roof/Horizontal = 9 (Used often in loops)
        constexpr int numTotalSurfaces = 9;
        constexpr int numVerticalSurfaces = 8;

        // // Surface Azimuths in radians: S, SE, E, NE, N, NW, W, SW
        // static constexpr std::array<double, 8> SurfaceAzimuths = { 
        //     0, -PI / 4, -PI / 2, -3 * PI / 4, PI, 3 * PI / 4, PI / 2, PI / 4 
        // };



        //// --- Unit Conversions ---
        constexpr double kWh2MJ = 3.6;              // 1 kWh = 3.6 MJ
        constexpr double MJ2kWh = 1.0 / 3.6;
        constexpr double MJ2Wh = 277.777778; // 1 MJ = 277.78 Wh
        constexpr double W2kW = 0.001;

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
