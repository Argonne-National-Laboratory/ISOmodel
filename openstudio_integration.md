# OpenStudio integration

## Conditional compilation

Conditional compilation for standalone and OpenStudio specific code is controlled by the `ISOMODEL_STANDALONE` preprocessor definition which is set in the standalone CMakeLists.txt with this command: `add_definitions(-DISOMODEL_STANDALONE)`.

Here are some examples of wrapping standalone and OpenStudio specific code:

```
#!c++

#ifdef ISOMODEL_STANDALONE
// Standalone specific code.
#endif
```

```
#!c++

#ifdef ISOMODEL_STANDALONE
// Standalone specific code.
#else
// OpenStudio specific code.
#endif
```

```
#!c++

#ifndef ISOMODEL_STANDALONE
// OpenStudio specific code.
#endif
```

## Source files

### Standalone source locations

- Source: IsoModel/src
- Tests: IsoModel/src/Test
- Test data: IsoModel/test\_data

### OpenStudio source location

- Source: openstudiocore/src/isomodel
- Tests: openstudiocore/src/isomodel/Test
- Test data: openstudiocore/resources/isomodel

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
