project(isomodel_project)
set(target_name isomodel)

cmake_minimum_required(VERSION 3.0)

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
  ForwardTranslator.cpp
  ForwardTranslator.hpp
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

if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
else()
  set(COMPILE_FLAGS "-std=c++11")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS})  
endif()


find_package(Boost 1.55.0 REQUIRED COMPONENTS system filesystem program_options)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIB_DIR})

set(${target_name}_depends
  ${Boost_LIBRARIES}
)

find_package(GTest REQUIRED)
set(unit_test_depends
  ${GTEST_LIBRARIES}
  ${Boost_LIBRARIES}
)

set(benchmark_depends
  ${Boost_LIBRARIES}
)

add_definitions(-DISOMODEL_STANDALONE)

set (exec_name isomodel_standalone)
add_executable(${exec_name} ${${target_name}_src} ${${target_name}_standalone})
target_link_libraries(${exec_name} ${${target_name}_depends})

add_executable(isomodel_unit_tests ${${target_name}_src} ${${target_name}_test})
target_include_directories(isomodel_unit_tests PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(isomodel_unit_tests ${unit_test_depends})

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


add_library(${library_name} SHARED ${${target_name}_src})
target_link_libraries(${library_name} ${${target_name}_depends})

if (MSVC) 
 target_compile_options(${library_name} PRIVATE "-Dopenstudio_isomodel_EXPORTS")
endif()
