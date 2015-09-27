# OpenStudio integration

## Source files

### Shared source files

- Building.cpp
- Building.hpp
- Cooling.cpp
- Cooling.hpp
- EpwData.cpp
- EpwData.hpp
- Heating.cpp
- Heating.hpp
- HourlyModel.cpp
- HourlyModel.hpp
- ISOModelAPI.hpp
- ISOResults.cpp
- ISOResults.hpp
- Lighting.cpp
- Lighting.hpp
- Location.hpp
- Location.cpp
- mainpage.hpp (Doxygen mainpage)
- MonthlyModel.cpp
- MonthlyModel.hpp
- PhysicalQuantities.cpp
- PhysicalQuantities.hpp
- Population.cpp
- Population.hpp
- Simulation.cpp
- Simulation.hpp
- SimulationSettings.cpp
- SimulationSettings.hpp
- SolarRadiation.cpp
- SolarRadiation.hpp
- Structure.cpp
- Structure.hpp
- TimeFrame.cpp
- TimeFrame.hpp
- UserModel.cpp
- UserModel.hpp
- Ventilation.hpp
- Ventilation.cpp
- WeatherData.cpp
- WeatherData.hpp
- Test/HourlyModel\_Gtest.cpp
- Test/ISOModelFixture.cpp
- Test/ISOModelFixture.hpp
- Test/MonthlyModel\_GTest.cpp
- Test/Properties\_GTest.cpp
- Test/SolarRadiation\_GTest.cpp
- Test/TimeFrame\_GTest.cpp
- Test/UserModel\_GTest.cpp

### Standalone only source files

- CMakeLists.txt (standalone version)
- EndUses.hpp (stands in for OpenStudio's EndUses class)
- Matrix.hpp (stands in for OpenStudio's Matrix class)
- standalone\_main.cpp
- Vector.hpp (stands in for OpenStudio's Vector class).
- Test/ISOModel\_Benchmark.cpp
- Test/ISOModel\_GTest.cpp
- Test/solar_debug.cpp

### OpenStudio only source files

- CMakeLists.txt (OpenStudio version)
- ForwardTranslator.cpp
- ForwardTranslator.hpp
- ISOModel.i (swig)
- Test/ForwardTranslator\_GTest.cpp
