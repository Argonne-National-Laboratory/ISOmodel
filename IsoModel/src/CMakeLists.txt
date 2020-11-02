project(isomodel_project)
set(target_name isomodel)

option(BUILD_RUBY_BINDINGS "Build Ruby bindings" ON)
option(BUILD_PYTHON_BINDINGS "Build Ruby bindings" OFF)
option(BUILD_CSHARP_BINDINGS "Build C# bindings" OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")

cmake_minimum_required(VERSION 3.14.0)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

include("${PROJECT_SOURCE_DIR}/ConanInstall.cmake")
include("${PROJECT_SOURCE_DIR}/FetchRubyMinGW.cmake")
include("${PROJECT_SOURCE_DIR}/ProjectMacros.cmake")

FetchRubyMinGW()

# A macro to find a conan related value especially when using multi-config builds (MSVC)
# But it also works with single-config builds
macro(FindValue ValueName)
  set(LocalVar "")
  set(LocalVar $<$<CONFIG:Debug>:${${ValueName}_DEBUG}>$<$<CONFIG:Release>:${${ValueName}_RELEASE}>$<$<CONFIG:RelWithDebInfo>:${${ValueName}_RELWITHDEBINFO}>$<$<CONFIG:MinSizeRel>:${${ValueName}_MINSIZEREL}>
  )
#  list(JOIN LocalVar "" LocalVar)
  string(STRIP ${LocalVar} LocalVar)
  set(CURRENT_${ValueName} $<IF:$<BOOL:${LocalVar}>,${LocalVar},${${ValueName}}>)
  # For debug purposes
   message(STATUS "Found '${ValueName}' as '${CURRENT_${ValueName}}'")
endmacro()

# So instead, we use a generator expression to find the right value. At least it makes sense, and should be robust.
FindValue(CONAN_SWIG_ROOT)

set(SWIG_EXECUTABLE "${CURRENT_CONAN_SWIG_ROOT}/bin/swig")

# The conan-provided binary has a built-in swiglib (`swig -swiglib`) that points to the build box on which it was built, which is problematic for us.
set(SWIG_LIB "${CURRENT_CONAN_SWIG_ROOT}/bin/swiglib")

if(BUILD_RUBY_BINDINGS)
  get_target_property(RUBY_INCLUDE_DIRS CONAN_PKG::openstudio_ruby INTERFACE_INCLUDE_DIRECTORIES)
endif()

set(${target_name}_test
  Test/HourlyModel_GTest.cpp
  Test/ISOModelFixture.cpp
  Test/ISOModelFixture.hpp
  Test/ISOModel_GTest.cpp
  Test/MonthlyModel_GTest.cpp
  Test/Properties_GTest.cpp
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
  Building.cpp
  Building.hpp
  Cooling.cpp
  Cooling.hpp
  EndUses.hpp
  EpwData.cpp
  EpwData.hpp
  Heating.cpp
  Heating.hpp
  HourlyModel.cpp
  HourlyModel.hpp
  ISOModelAPI.hpp
  Lighting.cpp
  Lighting.hpp
  Location.cpp
  Location.hpp
  Matrix.hpp
  MonthlyModel.cpp
  MonthlyModel.hpp
  PhysicalQuantities.cpp
  PhysicalQuantities.hpp
  Population.cpp
  Population.hpp
  Properties.cpp
  Properties.hpp
  Simulation.cpp
  Simulation.hpp
  SimulationSettings.cpp
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
  Ventilation.cpp
  Ventilation.hpp
  WeatherData.cpp
  WeatherData.hpp
)

set (library_name isomodel)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_SKIP_RPATH true)
endif()

set(${target_name}_depends
  CONAN_PKG::boost
)

set(benchmark_depends
  CONAN_PKG::boost
)

set(unit_test_depends
  CONAN_PKG::boost
  CONAN_PKG::gtest
)

add_definitions(-DISOMODEL_STANDALONE)

set (exec_name isomodel_standalone)
add_executable(${exec_name} ${${target_name}_src} ${${target_name}_standalone})
target_link_libraries(${exec_name} ${${target_name}_depends})

add_executable(isomodel_unit_tests ${${target_name}_src} ${${target_name}_test})
target_include_directories(isomodel_unit_tests PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(isomodel_unit_tests ${unit_test_depends} ${CMAKE_THREAD_LIBS_INIT})

add_executable(isomodel_benchmark ${${target_name}_src} ${${target_name}_benchmark})
target_link_libraries(isomodel_benchmark ${benchmark_depends})

add_executable(solar_debug ${${target_name}_src} ${${target_name}_solar_debug})
target_link_libraries(solar_debug ${${target_name}_depends})

# define USE_NEW_BUILDING_PARAMS if we are compiling the unit test target
# this allows use to test parsing the as yet unused parameters
set_property(
  TARGET isomodel_unit_tests
  PROPERTY COMPILE_DEFINITIONS USE_NEW_BUILDING_PARAMS
)

add_custom_command(TARGET isomodel_unit_tests
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                     ${CMAKE_SOURCE_DIR}/../test_data $<TARGET_FILE_DIR:isomodel_unit_tests>/test_data
                   )


add_library(${library_name} STATIC ${${target_name}_src})
target_link_libraries(${library_name} ${${target_name}_depends})

if (MSVC)
  target_compile_options(${library_name} PRIVATE "-Dopenstudio_isomodel_EXPORTS")
endif()

MAKE_SWIG_TARGET(IsoModel isomodel "${CMAKE_CURRENT_SOURCE_DIR}/IsoModel.i" "${CMAKE_CURRENT_SOURCE_DIR}/IsoModel.i" ${library_name} "")
