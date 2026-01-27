#ifndef ISOMODEL_CONSTANTS_HPP
#define ISOMODEL_CONSTANTS_HPP

#include <array>
#include <limits>
#include <numbers>
#include <string_view>

namespace openstudio::isomodel {

// flag to turn on debug printing of many intermediate variables to stdout
// constexpr bool DEBUG_ISO_MODEL_SIMULATION = false;

// Replaced #define maxDouble and minDouble with inline use of
// std::numeric_limits using e.g. std::numeric_limits<double>::epsilon() and
// std::numeric_limits<double>::infinity()

//// --- Math & Physics ---
inline constexpr double PI = std::numbers::pi;
constexpr double SAFE_EPSILON = 1e-15; // Small value to prevent division by zero
constexpr double UNITY_FRACTION = 1.0;  // Unity value for fractions    

// Physical Constants
constexpr double RHO_AIR = 1.22521;          // Density of air (kg/m3)
constexpr double CP_AIR = 0.001012;          // Specific heat capacity of air in MJ/kg*K
constexpr double RHO_CP_AIR = RHO_AIR * CP_AIR; // = 0.001239 MJ/m3/K
// Volumetric heat capacity of air in wh/m3K = rho*cp in MJ/m3K * 1000000 J/MJ / 3600 s/h = Wh/m3K 
constexpr double RHO_CP_AIR_IN_WATT_HOURS = RHO_CP_AIR * 1000000.0 / 3600; 

constexpr double RHO_CP_WATER = 4.1813; // Volumetric heat capacity of water (MJ/m3/K)

// ventilation physics constants from ISO 15242 6.7.1
// based on Q = C * (dP)^0.667
constexpr double STACK_FACTOR = 0.0146; // Physics constant for qStack
constexpr double EFFECTIVE_STACK_HEIGHT_FRACTION =
    0.5; // Effective stack height is 50% of zone height
constexpr double WIND_FACTOR = 0.0769; // Physics constant for qWind
constexpr double Q_INFIL_STACT_FRACTION =
    0.5; // coefficient for infiltration from stack effect
constexpr double Q_INFIL_WIND_FRACTION =
    2.0 / 3.0; // coefficient for infiltration from wind effect

// This constant converts the physics of thermal buoyancy (stack effect) into a
// flow rate relative to the leakage measured at 4 Pa ($Q_{4Pa}$).
//  Q_4Pa = Q_50Pa * (4/50)^0.667 = Q_50Pa * 0.28 but reduce by a factor to
//  account for the fact that the wind pressure is not always perpendicular to
//  the surface and other empirical factors.

// ISO 13790 12.3.1.2 Table 12 constants for heat capacity categories
constexpr double VERY_HEAVY = 370.0;
constexpr double HEAVY = 260.0;
constexpr double MEDIUM = 165.0;
constexpr double LIGHT = 110.0;
constexpr double VERY_LIGHT = 80.0;

// from usermodel.hpp
// Defined as const char* for efficiency, but fully compatible with std::string
// comparisons.
inline constexpr std::string_view GAS = "gas";
inline constexpr std::string_view ELECTRIC = "electric";
inline constexpr std::string_view MECHANICAL = "mechanical";
inline constexpr std::string_view NATURAL = "natural";
inline constexpr std::string_view COMBINED = "combined";
inline constexpr std::string_view NONE = "none";
inline constexpr std::string_view SIMPLE = "simple";
inline constexpr std::string_view ADVANCED = "advanced";

// --- Time Constants ---

constexpr int MONTHS_IN_YEAR = 12;
constexpr int HOURS_IN_DAY = 24;
constexpr int HOURS_IN_WEEK = 168;
constexpr int DAYS_IN_WEEK = 7;
constexpr int DAYS_IN_YEAR = 365;
// constexpr int HOURS_IN_YEAR = 8760;
constexpr int HOURS_IN_YEAR = DAYS_IN_YEAR * HOURS_IN_DAY;
constexpr int SECONDS_IN_HOUR = 3600;

// Start hour for a standard weekday in EECALC
constexpr int WEEKDAY_START = 7;

// Constants
inline constexpr std::array<double, 12> DAYS_IN_MONTH = {31, 28, 31, 30, 31, 30,
                                                       31, 31, 30, 31, 30, 31};

inline constexpr std::array<double, 12> HOURS_IN_MONTH = {
    744, 672, 744, 720, 744, 720, 744, 744, 720, 744, 720, 744};

inline constexpr std::array<double, 12> MEGASECONDS_IN_MONTH = {
    2.6784, 2.4192, 2.6784, 2.592,  2.6784, 2.592,
    2.6784, 2.6784, 2.592,  2.6784, 2.592,  2.6784};

inline constexpr std::array<double, 12> MONTH_FRACTION_OF_YEAR = {
    0.0849315068493151, 0.0767123287671233, 0.0849315068493151,
    0.0821917808219178, 0.0849315068493151, 0.0821917808219178,
    0.0849315068493151, 0.0849315068493151, 0.0821917808219178,
    0.0849315068493151, 0.0821917808219178, 0.0849315068493151};

// Cumulative hours at the end of each month (0 to 8760)
inline constexpr std::array<int, 13> MONTH_END_HOURS = {
    0, 744, 1416, 2160, 2880, 3624, 4344, 5088, 5832, 6552, 7296, 8016, 8760};

// --- Geometry & Directions ---
// 8 Compass directions (N, NE, E, SE, S, SW, W, NW)
constexpr int NUM_COMPASS_DIRECTIONS = 8;
// 8 Compass directions + 1 Roof/Horizontal = 9 (Used often in loops)
constexpr int NUM_TOTAL_SURFACESs = 9;
constexpr int NUM_VERTICAL_SURFACES = 8;

// Surface Azimuths in radians: S, SE, E, NE, N, NW, W, SW
inline constexpr std::array<double, 8> SURFACE_AZIMUTHS = {
    0, -PI / 4, -PI / 2, -3 * PI / 4, PI, 3 * PI / 4, PI / 2, PI / 4};

//// --- Unit Conversions ---
constexpr double KILOWATTHOURS_TO_MEGAJOULES = 3.6; // 1 kWh = 3.6 MJ
constexpr double MEGAJOULES_TO_KILOWATTHOURS = 1.0 / KILOWATTHOURS_TO_MEGAJOULES;
constexpr double MEGAJOULES_TO_WATTHOURS = 1000*MEGAJOULES_TO_KILOWATTHOURS; // 1 MJ = 277.78 Wh
constexpr double KILOWATTS_TO_WATTS = 1000.0;
constexpr double WATTS_TO_KILOWATTS = 0.001;
constexpr double LITERS_PER_SECOND_TO_METERS3_PER_HOUR = 3.6;        // Liters/sec to m3/h
constexpr double DEGREES_PER_HOUR = 15.0; // Earth rotation
constexpr double LITERS_TO_M3 = 1000.0;
constexpr double KILOJOULE_TO_MEGAJOULE = 1000.0;
constexpr double MEGASECONDS_TO_SECONDS = 1000000.0;

// ISO 15242 Annex D Table D.1: Total air leakage at 4Pa
// 0.19 is conversion from n50 to q_ve_4Pa with exponent 0.667  Move to
// Constants.hpp
constexpr double N50_TO_Q4 = 0.19;

//// --- ISO 13790 Constants ---
// Solar heat gain coefficient for internal gains
constexpr double n_si_coeff = 0.9;

// from ventilation calcs in MonthlyModel.cpp
constexpr double N_SW_COEFF = 0.14;

// default ground reflectance
constexpr double DEFAULT_GROUND_REFLECTANCE = 0.14;

// Shading device factors (1=None, 2=Internal, 3=External)
inline constexpr std::array<double, 3> WIN_SDF_TABLE = {0.5, 0.35, 1.0};
// Form factors given in ISO 13790, 11.4.6 (0.5 for wall, 1.0 for unshaded roof)
inline constexpr std::array<double, 9> ENV_FORM_FACTORS = {
    0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0};

constexpr double SHGC_CLEAR_GLASS = 0.87;

// Simulation Defaults
constexpr double DEFAULT_INITIAL_TEMP = 20.0; // Degrees C
constexpr double ISO_SKY_TEMP_DIFF = 11.0;    // K (Intermediate zones)
constexpr double ISO_WIN_EXT_RAD_COEFF = 5.0; // W/m2K
constexpr double LIGHTING_LEVEL_COEFF = 53.0;  // Empirical constant for dayLIGHTing
constexpr double H_MS_FACTOR = 1.2; // Relation between h_ms and h_ri
constexpr double MIN_VENT_ZONE_HEIGHT = 0.1; // meters

// Monthly Model Constants
constexpr double MIN_INFILTRATION_FLOW = 0.001; // m3/h/m2
constexpr double MIN_DEMAND_FRACTION = 0.1;     // Minimum fraction of yearly demand
constexpr double BEM_SIMPLE_ADJUSTMENT = 0.5;   // K
constexpr double BEM_ADVANCED_ADJUSTMENT = 1.0; // K



constexpr double DEFAULT_DH_NETWORK_EFF = 0.9;
constexpr double DEFAULT_DH_SYS_EFF = 0.87;
constexpr double DEFAULT_DC_COP = 5.5;

} // namespace openstudio::isomodel

#endif // ISOMODEL_CONSTANTS_HPP
