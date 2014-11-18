set(target_name isomodel)

cmake_minimum_required(VERSION 3.0)

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

set (exec_name isomodel_standalone)
add_executable(${exec_name} ${${target_name}_src})
target_link_libraries(${exec_name} ${${target_name}_depends})

add_library(${library_name} SHARED ${${target_name}_src})
target_link_libraries(${library_name} ${${target_name}_depends})

if (MSVC) 
 target_compile_options(${library_name} PRIVATE "-Dopenstudio_isomodel_EXPORTS")
endif()
