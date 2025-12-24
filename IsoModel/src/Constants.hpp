#ifndef ISOMODEL_CONSTANTS_HPP
#define ISOMODEL_CONSTANTS_HPP

#include <limits>
#include <array>

//flag to turn on debug printing of many intermediate variables to stdout
// #define DEBUG_ISO_MODEL_SIMULATION false


namespace openstudio {
    namespace isomodel {

        constexpr bool debugIsoModelSimulation = false;

        // Replaces #define maxDouble and minDouble
        constexpr double maxDouble = std::numeric_limits<double>::max();
        constexpr double minDouble = std::numeric_limits<double>::min();   

        //// --- Math & Physics ---
        constexpr double PI = 3.14159265358979323846;
        constexpr double smallEpsilon = 1e-15;  // Used for safe division/avoiding zero


        // Pyhsical Constants
        // Volumetric heat capacity of air (MJ/m3/K)
        // Derived from: rho (1.22521 kg/m3) * cp (1.012 kJ/kg*K) / 1000 kJ/MJ
        constexpr double rhoCpAir = 1.22521 * 0.001012; // = 0.001239 MJ/m3/K
        constexpr double rhoCpAirWh = rhoCpAir*1000000.0/3600; // Volumetric heat capacity of air ~1200 J/m3K / 3600 = 0.33-0.34 Wh/m3K
        constexpr double rhoCpWater = 4.1813;  // Volumetric heat capacity of water (MJ/m3/K)
       
        constexpr double windFactor = 0.0769; // Physics constant for qWind
        constexpr double stackFactor = 0.0146; // Physics constant for qStack


        // from usermodel.hpp
        // Defined as const char* for efficiency, but fully compatible with std::string comparisons.
        constexpr const char* GAS = "gas";
        constexpr const char* ELECTRIC = "electric";
        constexpr const char* MECHANICAL = "mechanical";
        constexpr const char* NATURAL = "natural";
        constexpr const char* COMBINED = "combined";
        constexpr const char* NONE = "none";
        constexpr const char* SIMPLE = "simple";
        constexpr const char* ADVANCED = "advanced";


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
        constexpr int eecalcWeekdayStart = 7;

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
        // Cumulative hours at the end of each month (0 to 8760)
        constexpr int monthEndHours[] = { 
            0, 744, 1416, 2160, 2880, 3624, 4344, 5088, 5832, 6552, 7296, 8016, 8760 
        };

        // --- Geometry & Directions ---
        // 8 Compass directions (N, NE, E, SE, S, SW, W, NW)
        constexpr int numCompassDIrections = 8;
        // 8 Compass directions + 1 Roof/Horizontal = 9 (Used often in loops)
        constexpr int numTotalSurfaces = 9;
        constexpr int numVerticalSurfaces = 8;

        // Surface Azimuths in radians: S, SE, E, NE, N, NW, W, SW
        static constexpr std::array<double, 8> SurfaceAzimuths = { 
            0, -PI / 4, -PI / 2, -3 * PI / 4, PI, 3 * PI / 4, PI / 2, PI / 4 
        };

        //// --- Unit Conversions ---
        constexpr double kWh2MJ = 3.6;              // 1 kWh = 3.6 MJ
        constexpr double MJ2kWh = 1.0 / 3.6;
        constexpr double MJ2Wh = 277.777778; // 1 MJ = 277.78 Wh
        constexpr double W2kW = 0.001;

        //// --- ISO 13790 Constants ---
        // Solar heat gain coefficient for internal gains
        constexpr double n_si_coeff = 0.9;

        // from ventilation calcs in MonthlyModel.cpp
        constexpr double n_sw_coeff = 0.14;

        // default ground reflectance
        constexpr double defaultGroundReflectance = 0.14;

        // Shading device factors (1=None, 2=Internal, 3=External)
        constexpr double winSDFTable[] = { 0.5, 0.35, 1.0 };;
        // Form factors given in ISO 13790, 11.4.6 (0.5 for wall, 1.0 for unshaded roof)
        constexpr double envFormFactors[] = { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0 };



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
