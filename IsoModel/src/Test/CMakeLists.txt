
set(test_src
  HourlyModel_GTest.cpp
  ISOModelFixture.cpp
  ISOModelFixture.hpp
  ISOModel_GTest.cpp
  MonthlyModel_GTest.cpp
  Properties_GTest.cpp
  SolarRadiation_GTest.cpp
  TimeFrame_GTest.cpp
  UserModel_GTest.cpp
)

set(benchmark_src
  ISOModel_Benchmark.cpp
)

set(solar_debug_src
  solar_debug.cpp
)

add_executable(isomodel_unit_tests ${test_src})

target_include_directories(isomodel_unit_tests PUBLIC ${GTEST_INCLUDE_DIRS})

target_link_libraries(isomodel_unit_tests
  isomodel_lib
  CONAN_PKG::gtest
  ${CMAKE_THREAD_LIBS_INIT}
)

# define USE_NEW_BUILDING_PARAMS if we are compiling the unit test target
# this allows use to test parsing the as yet unused parameters
set_property(
  TARGET isomodel_unit_tests
  PROPERTY COMPILE_DEFINITIONS USE_NEW_BUILDING_PARAMS
)

add_custom_command(TARGET isomodel_unit_tests
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/../test_data $<TARGET_FILE_DIR:isomodel_unit_tests>/test_data
)

add_executable(isomodel_benchmark ${benchmark_src})
target_link_libraries(isomodel_benchmark isomodel_lib)

add_executable(solar_debug ${solar_debug_src})
target_link_libraries(solar_debug isomodel_lib)

gtest_discover_tests(isomodel_unit_tests
  WORKING_DIRECTORY $<TARGET_FILE_DIR:isomodel_unit_tests>
)
