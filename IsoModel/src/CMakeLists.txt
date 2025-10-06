cmake_minimum_required(VERSION 3.5)

# Set policy to ignore warning about FindBoost module
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
set(CXX_STANDARD 20
target_compile_features(${target_name} PUBLIC cxx_std_20))

set(Boost_NO_WARN_NEW_VERSIONS 1)

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



# Fetch GoogleTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0  # or whatever version you prefer
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Make GoogleTest available
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0
)
# Fetch the content without building
FetchContent_GetProperties(yaml-cpp)
if(NOT yaml-cpp_POPULATED)
  FetchContent_Populate(yaml-cpp)
  
  message("Downloading and compiling yaml-cpp")
  # Now modify the CMakeLists.txt
  file(READ "${yaml-cpp_SOURCE_DIR}/CMakeLists.txt" YAML_CPP_CMAKE_CONTENT)
  string(REPLACE "cmake_minimum_required(VERSION 3.4)" 
                 "cmake_minimum_required(VERSION 3.20)" 
                 YAML_CPP_CMAKE_CONTENT "${YAML_CPP_CMAKE_CONTENT}")
  file(WRITE "${yaml-cpp_SOURCE_DIR}/CMakeLists.txt" "${YAML_CPP_CMAKE_CONTENT}")
  
  # Add the subdirectory
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
else()
  # If already populated, just make it available
  FetchContent_MakeAvailable(yaml-cpp)
endif()







set (library_name isomodel)

if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS} -pthread)
  # # required for googletest
  # FIND_PACKAGE ( Threads REQUIRED )
  # Threads are already found by GoogleTest
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(CMAKE_SKIP_RPATH true)
endif()

find_package(Boost 1.55.0 REQUIRED COMPONENTS system filesystem program_options)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIB_DIR})

set(${target_name}_depends
  ${Boost_LIBRARIES}
)

# find_package(GTest REQUIRED)
# set(unit_test_depends
  # ${GTEST_LIBRARIES}
  # ${Boost_LIBRARIES}
# )

# set(benchmark_depends
  # ${Boost_LIBRARIES}
# )

# Remove the find_package(GTest REQUIRED) line since we're using FetchContent
# set(unit_test_depends ${GTEST_LIBRARIES} ${Boost_LIBRARIES})
set(unit_test_depends GTest::gtest GTest::gtest_main ${Boost_LIBRARIES})
set(benchmark_depends ${Boost_LIBRARIES})


add_definitions(-DISOMODEL_STANDALONE)

# Standalone executable
set (exec_name isomodel_standalone)
add_executable(${exec_name} ${${target_name}_src} ${${target_name}_standalone})
target_link_libraries(${exec_name} ${${target_name}_depends})
target_link_libraries(${exec_name} yaml-cpp::yaml-cpp)

# Unit tests executable
# add_executable(isomodel_unit_tests ${${target_name}_src} ${${target_name}_test})
# target_include_directories(isomodel_unit_tests PUBLIC ${GTEST_INCLUDE_DIRS})
# target_link_libraries(isomodel_unit_tests ${unit_test_depends} ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries(isomodel_unit_tests yaml-cpp::yaml-cpp)

add_executable(isomodel_unit_tests ${${target_name}_src} ${${target_name}_test})
# Remove the manual include directories for GTest
# target_include_directories(isomodel_unit_tests PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(isomodel_unit_tests ${unit_test_depends})
target_link_libraries(isomodel_unit_tests yaml-cpp::yaml-cpp)


# Benchmark executable
add_executable(isomodel_benchmark ${${target_name}_src} ${${target_name}_benchmark})
target_link_libraries(isomodel_benchmark ${benchmark_depends})
target_link_libraries(isomodel_benchmark yaml-cpp::yaml-cpp)

# Solar debug executable
add_executable(solar_debug ${${target_name}_src} ${${target_name}_solar_debug})
target_link_libraries(solar_debug ${${target_name}_depends})
target_link_libraries(solar_debug yaml-cpp::yaml-cpp)

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
target_link_libraries(${library_name}  yaml-cpp::yaml-cpp)


if (MSVC)
 target_compile_options(${library_name} PRIVATE "-Dopenstudio_isomodel_EXPORTS")
endif()
