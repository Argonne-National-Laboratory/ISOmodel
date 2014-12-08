project(isomodel_project)
set(target_name isomodel)

cmake_minimum_required(VERSION 3.0)

set(${target_name}_test
	Test/ISOModel_GTest.cpp
)

set(${target_name}_standalone
	standalone_main.cpp
)

set(${target_name}_src
	Building.hpp
	Cooling.hpp
	EpwData.hpp
	ForwardTranslator.hpp
	Heating.hpp
    ISOHourly.hpp
	ISOModelAPI.hpp
	Lighting.hpp
	Location.hpp
	Population.hpp
	SimModel.hpp
	SolarRadiation.hpp
	Structure.hpp
	TimeFrame.hpp
	UserModel.hpp
	Ventilation.hpp
	WeatherData.hpp
	EndUses.hpp
	Matrix.hpp
	Vector.hpp
	Properties.hpp
	Building.cpp
	Cooling.cpp
	EpwData.cpp
	ForwardTranslator.cpp
	Heating.cpp
    ISOHourly.cpp
	Lighting.cpp
	Location.cpp
	Population.cpp
	SimModel.cpp
	SolarRadiation.cpp
	Structure.cpp
	TimeFrame.cpp
	UserModel.cpp
	Ventilation.cpp
	WeatherData.cpp
	Properties.cpp
)


set (library_name isomodel)

if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
else()
  set(COMPILE_FLAGS "-std=c++11")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS})  
endif()


find_package(Boost 1.55.0 REQUIRED COMPONENTS system filesystem)
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


set (exec_name isomodel_standalone)
add_executable(${exec_name} ${${target_name}_src} ${${target_name}_standalone})
target_link_libraries(${exec_name} ${${target_name}_depends})

add_executable(isomodel_unit_tests ${${target_name}_src} ${${target_name}_test})
target_include_directories(isomodel_unit_tests PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(isomodel_unit_tests ${unit_test_depends})

# define USE_NEW_BUILDING_PARAMS if we are compiling the unit test target
# this allows use to test parsing the as yet unused parameters 
set_property(
	TARGET isomodel_unit_tests 
	PROPERTY COMPILE_DEFINITIONS USE_NEW_BUILDING_PARAMS
)


add_library(${library_name} SHARED ${${target_name}_src})
target_link_libraries(${library_name} ${${target_name}_depends})

if (MSVC) 
 target_compile_options(${library_name} PRIVATE "-Dopenstudio_isomodel_EXPORTS")
endif()
