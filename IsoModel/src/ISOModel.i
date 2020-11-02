#ifndef ISOMODEL_I
#define ISOMODEL_I

#ifdef SWIGPYTHON
%module isomodel
#endif

#ifdef SWIGRUBY
%begin %{
// this must be included early because ruby.h 2.0.0  breaks Qt
//#include <QByteArray>
//#include <QString>

// xkeycheck.h emits warning 4505 if any C++ keyword is redefined, ruby.h does redefine many keywords
// for some reason our undefs below are not sufficient to avoid this warning
// Qt headers QByteArray and QString (now removed) included some workaround for this which I did not find
// we can use #define _ALLOW_KEYWORD_MACROS 1
// other option is to patch ruby.h and config.h to remove keyword redefinitions (e.g. #define inline __inline)
#ifdef _MSC_VER
  #define _ALLOW_KEYWORD_MACROS 1
#endif

%}

%header %{
// Must undo more of the damage caused by ruby/win32.h 2.0.0
#ifdef access
#undef access
#endif

#ifdef truncate
#undef truncate
#endif

#ifdef inline
#undef inline
#endif
%}

#endif

%include <stl.i>

#define ISOMODEL_API

%{
  #include <algorithm>

  #include "Building.hpp"
  #include "Cooling.hpp"
  #include "EndUses.hpp"
  #include "EpwData.hpp"
  #include "Heating.hpp"
  #include "HourlyModel.hpp"
  #include "ISOModelAPI.hpp"
  #include "ISOResults.hpp"
  #include "Lighting.hpp"
  #include "Location.hpp"
  #include "Matrix.hpp"
  #include "MonthlyModel.hpp"
  #include "PhysicalQuantities.hpp"
  #include "Population.hpp"
  #include "Properties.hpp"
  #include "Simulation.hpp"
  #include "SimulationSettings.hpp"
  #include "SolarRadiation.hpp"
  #include "TimeFrame.hpp"
  #include "UserModel.hpp"
  #include "Vector.hpp"
  #include "Ventilation.hpp"
  #include "WeatherData.hpp"

  using namespace openstudio;
  using namespace openstudio::isomodel;
%}

%include "UserModel.hpp"

#endif //ISOMODEL_I
