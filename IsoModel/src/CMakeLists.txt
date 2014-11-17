set(target_name isomodel)

cmake_minimum_required(VERSION 3.0)

set(${target_name}_src
	isomodel/Building.hpp
	isomodel/Cooling.hpp
	isomodel/EpwData.hpp
	isomodel/ForwardTranslator.hpp
	isomodel/Heating.hpp
    isomodel/ISOHourly.hpp
	isomodel/ISOModelAPI.hpp
	isomodel/Lighting.hpp
	isomodel/Location.hpp
	isomodel/Population.hpp
	isomodel/SimModel.hpp
	isomodel/SolarRadiation.hpp
	isomodel/Structure.hpp
	isomodel/TimeFrame.hpp
	isomodel/UserModel.hpp
	isomodel/Ventilation.hpp
	isomodel/WeatherData.hpp
	utilities/data/EndUses.hpp
	utilities/data/Matrix.hpp
	utilities/data/Vector.hpp
	Building.cpp
	Cooling.cpp
	EndUses.cpp
	EpwData.cpp
	ForwardTranslator.cpp
	Heating.cpp
    ISOHourly.cpp
	Lighting.cpp
	Location.cpp
	Matrix.cpp
	Population.cpp
	SimModel.cpp
	SolarRadiation.cpp
	Structure.cpp
	TimeFrame.cpp
	UserModel.cpp
	Vector.cpp
	Ventilation.cpp
	WeatherData.cpp
)

set(${target_name}_main 
  	main.cpp
)

add_definitions( -DBOOST_ALL_NO_LIB )

if(MSVC)
  set(Boost_USE_STATIC_LIBS ON)
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

set (library_name isomodel)
add_library(${library_name} SHARED ${${target_name}_src})
target_link_libraries(${library_name} ${${target_name}_depends})
