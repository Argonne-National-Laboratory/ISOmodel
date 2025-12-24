cmake_minimum_required(VERSION 3.20)

# Set policy to ignore warning about FindBoost module since we are removing it
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()

# Set policy to allow FetchContent_Populate
if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

include(FetchContent)

project(isomodel_project)
set(target_name isomodel)

# ---------------------------------------------------------------------------
# OPTIMIZATION: Set Default Build Type to Release
# ---------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Set C++ Standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# SOURCE DEFINITIONS
# ---------------------------------------------------------------------------
set(${target_name}_test
  Test/HourlyModel_GTest.cpp
  Test/ISOModelFixture.cpp
  Test/ISOModelFixture.hpp
  Test/ISOModel_GTest.cpp
  Test/MonthlyModel_GTest.cpp
  Test/SolarRadiation_GTest.cpp
  Test/TimeFrame_GTest.cpp
  Test/UserModel_GTest.cpp
)

set(${target_name}_benchmark
  Test/ISOModel_Benchmark.cpp
)

set(${target_name}_standalone
  standalone_main.cpp
)

set(${target_name}_solar_debug
  Test/solar_debug.cpp
)

set(${target_name}_src
  # Building.cpp
  Building.hpp
  # Cooling.cpp
  Cooling.hpp
  EndUses.hpp
  EpwData.cpp
  EpwData.hpp
  # Heating.cpp
  Heating.hpp
  HourlyModel.cpp
  HourlyModel.hpp
  ISOModelAPI.hpp
  ISOResults.cpp
  ISOResults.hpp
  # 
  # Lighting.cpp  <-- COMMENTED OUT TO FIX BUILD ERROR
  Lighting.hpp
  # Location.cpp
  Location.hpp
  Matrix.hpp
  MonthlyModel.cpp
  MonthlyModel.hpp
  MathHelpers.hpp
  # PhysicalQuantities.cpp
  PhysicalQuantities.hpp
  # Population.cpp
  Population.hpp
  # Simulation.cpp
  Simulation.hpp
  # SimulationSettings.cpp
  SimulationSettings.hpp
  SolarRadiation.cpp
  SolarRadiation.hpp
  Structure.cpp
  Structure.hpp
  TimeFrame.cpp
  TimeFrame.hpp
  UserModel.cpp
  UserModel.hpp
  Vector.hpp
  # Ventilation.cpp
  Ventilation.hpp
  # WeatherData.cpp
  WeatherData.hpp
)

# ---------------------------------------------------------------------------
# DEPENDENCIES
# ---------------------------------------------------------------------------
# Fetch GoogleTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch yaml-cpp
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0
)
FetchContent_GetProperties(yaml-cpp)
if(NOT yaml-cpp_POPULATED)
  FetchContent_Populate(yaml-cpp)
  file(READ "${yaml-cpp_SOURCE_DIR}/CMakeLists.txt" YAML_CPP_CMAKE_CONTENT)
  string(REPLACE "cmake_minimum_required(VERSION 3.4)" 
                 "cmake_minimum_required(VERSION 3.20)" 
                 YAML_CPP_CMAKE_CONTENT "${YAML_CPP_CMAKE_CONTENT}")
  file(WRITE "${yaml-cpp_SOURCE_DIR}/CMakeLists.txt" "${YAML_CPP_CMAKE_CONTENT}")
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
else()
  FetchContent_MakeAvailable(yaml-cpp)
endif()

set (library_name isomodel)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_SKIP_RPATH true)
endif()

# Removed all Boost find_package and dependency logic
set(${target_name}_depends "")

set(unit_test_depends GTest::gtest GTest::gtest_main)
set(benchmark_depends "")

add_definitions(-DISOMODEL_STANDALONE)

# ---------------------------------------------------------------------------
# OPTIMIZATION FLAGS
# ---------------------------------------------------------------------------
if(NOT MSVC)
  # -march=native: Optimizes for the local machine's CPU (AVX2/AVX512)
  # -ffast-math:   Allows faster floating point math (safe for this simulation)
  # -funroll-loops: Unrolls loops for speed
  set(PERF_FLAGS -march=native -ffast-math -funroll-loops)
else()
  set(PERF_FLAGS "")
endif()

# ---------------------------------------------------------------------------
# TARGETS
# ---------------------------------------------------------------------------

# 1. Standalone Executable
set (exec_name isomodel_standalone)
add_executable(${exec_name} ${${target_name}_src} ${${target_name}_standalone})
target_link_libraries(${exec_name} PRIVATE yaml-cpp::yaml-cpp)
target_compile_options(${exec_name} PRIVATE ${PERF_FLAGS})

# 2. Unit Tests Executable
add_executable(isomodel_unit_tests ${${target_name}_src} ${${target_name}_test})
target_link_libraries(isomodel_unit_tests PRIVATE ${unit_test_depends} yaml-cpp::yaml-cpp)
# Note: Unit tests generally don't use aggressive optimization to ensure stability,
# but you can add ${PERF_FLAGS} here if you want to test the optimized build.

set_property(
  TARGET isomodel_unit_tests
  PROPERTY COMPILE_DEFINITIONS USE_NEW_BUILDING_PARAMS
)

add_custom_command(TARGET isomodel_unit_tests
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${CMAKE_SOURCE_DIR}/../test_data $<TARGET_FILE_DIR:isomodel_unit_tests>/test_data
)

# 3. Benchmark Executable
add_executable(isomodel_benchmark ${${target_name}_src} ${${target_name}_benchmark})
target_link_libraries(isomodel_benchmark PRIVATE yaml-cpp::yaml-cpp)
target_compile_options(isomodel_benchmark PRIVATE ${PERF_FLAGS})

# 4. Solar Debug Executable
add_executable(solar_debug ${${target_name}_src} ${${target_name}_solar_debug})
target_link_libraries(solar_debug PRIVATE yaml-cpp::yaml-cpp)

# 5. Shared Library
add_library(${library_name} SHARED ${${target_name}_src})
target_link_libraries(${library_name} PRIVATE yaml-cpp::yaml-cpp)
target_compile_options(${library_name} PRIVATE ${PERF_FLAGS})

if (MSVC)
 target_compile_options(${library_name} PRIVATE "-Dopenstudio_isomodel_EXPORTS")
endif()