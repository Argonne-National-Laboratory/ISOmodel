<!-- Last edited 08-Jun-2026 -->

The isomodel code uses CMake for compilation. 

# Instructions for installing and compiling within Windows using Visual Studio 2026

As of 15-Dec-2025 Boost dependence has been removed and Google Test is downloaded automagically along with yaml stuff as part of the compile process.  This makes install and compilation much easier for both Windows and Linux based development.

If you plan to work from the publicly available, open source, ISOmodel git repository and keep your changes/additions open source, the terms of use allow use of VS 2026 community edition even if you are a commercial business or enterprise.  If you are a commercial company and plan on keeping those changes within your company or releasing in closed source software, you cannot use VS 2026 community edition and must have a licensed version VS 2026 Pro.  This code used to work with VS 2022 but since I updated to VS 2026 recently, I can only confirm these instructions work with VS 2026.

https://visualstudio.microsoft.com/vs/features/cplusplus/

### 1) Install Cmake > 4.2 

CMake > 4.2 is needed in order to properly detect 


https://github.com/Kitware/CMake/releases/download/v4.3.3/cmake-4.3.3-windows-x86_64.msi


or, for whatever the latest is:

https://cmake.org/download/

Install with the option “Add CMake to the PATH environment variable” checked.
This will prompt for admin login



### 2) Install Visual Studio (VS) 2026 

It's best to install CMake first so that the VS installer can find CMake when it installs.

Download VS from https://visualstudio.microsoft.com/vs/features/cplusplus/

This downloads the installer set up already for C++ development

After you install (it will take a while) make sure you run VS 2026 to be sure the install completed correctly and initial files are all set up right.  After starting, you can just exit it again.

### 3) Create the Visual Studio Project Files

Now, we need to create the .sln files

Open a command prompt that has cmake in the path.  If you want to compile from the command line (i.e. not through the GUI) then open up a Visual Studio command line from the Windows Menu Options.

Go to isomodel directory root (e.g. c:\git\isomodel\isomodel\) and make a bin directory for the binaries
```
cd C:\git\isomodel\isomodel
mkdir build
cd build
```

Now, if you are lucky, cmake will autodetect Visual Studio 26 and you can create the Visual Studio 2026 project files with the command line:
 
```
cmake ../src
```

If all goes well things will run, find the compiler, print some output, a maybe couple warnings about not finding python, and then finally end with the following lines (times will differ on your machine)

```
-- Configuring done (73.1s)
-- Generating done (1.1s)
-- Build files have been written to: C:/Git/ISOmodel/IsoModel/bin
```

Here is the full output on my computer
```c:\Git\ISOmodel\IsoModel\bin>cmake ../src
-- Building for: Visual Studio 18 2026
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- The C compiler identification is MSVC 19.51.36246.0
-- The CXX compiler identification is MSVC 19.51.36246.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/MSVC/14.51.36231/bin/Hostx64/x64/cl.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Could NOT find Python3 (missing: Python3_EXECUTABLE Interpreter)
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed
-- Looking for pthread_create in pthreads
-- Looking for pthread_create in pthreads - not found
-- Looking for pthread_create in pthread
-- Looking for pthread_create in pthread - not found
-- Found Threads: TRUE
-- Configuring done (76.8s)
-- Generating done (1.7s)
-- Build files have been written to: C:/Git/ISOmodel/IsoModel/bin

c:\Git\ISOmodel\IsoModel\bin>

```


If cmake terminates in an error cmake might not finding the Visual Studio compiler properly.

If you don’t see something like the following line at the top, then the Visual Studio compiler was not found (exact #’s will change as VS 2026 has updates)
```
-- Building for: Visual Studio 17 2022
And then farther down
-- The C compiler identification is MSVC 19.44.35217.0
-- The CXX compiler identification is MSVC 19.44.35217.0
```

To try to fix this problem you can tell cmake explicitly the version by specifically adding info on the command line with cmake.  

Try:
```
cmake ../src -G "Visual Studio 18 2026" 
```


### 4) Compiling the VS files from the command line

To compile from the command line using the VS tools you should open up a VS Studio Command Line.  Add the "-v" option for highly verbose output.  It's a lot, but you at least know the system is still compiling.  Add "-- /v:m" for less verbose output but something lets you see progress happening

```
cd ..

cmake --build bin --config Release -v
```
or for less verbose output
```
cmake --build bin --config Release -- /v:m
```

Most of the compiled code (isomodel.dll, isomodel.lib isomodel_benchmark.exe isomodel_standalone.exe, isomodel_unit_tests.exe, etc) is in the bin/Release directory.



# Running code

To test the code, go to the Release directory ( bin/Release) and run the code:

```
isomodel_unit_tests.exe

isomodel_benchmark.exe .

isomodel_standalone test_bldg.yaml

or 

isomodel_standalone test_bldg_schedules.yaml

```

### Unit tests

```
c:\Git\ISOmodel\IsoModel\build_yaml_merged\Release>isomodel_unit_tests.exe
[==========] Running 18 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 14 tests from ISOModelFixture
[ RUN      ] ISOModelFixture.HourlyModelTests
[       OK ] ISOModelFixture.HourlyModelTests (77 ms)
[ RUN      ] ISOModelFixture.HourlyModelScheduleTests
[       OK ] ISOModelFixture.HourlyModelScheduleTests (76 ms)
[ RUN      ] ISOModelFixture.MonthlyModelTests
[       OK ] ISOModelFixture.MonthlyModelTests (42 ms)
[ RUN      ] ISOModelFixture.SunPositionAndRadiationTests
[       OK ] ISOModelFixture.SunPositionAndRadiationTests (89 ms)
[ RUN      ] ISOModelFixture.TimeFrameMonthLengthTest
[       OK ] ISOModelFixture.TimeFrameMonthLengthTest (0 ms)
[ RUN      ] ISOModelFixture.TimeFrameHourTests
[       OK ] ISOModelFixture.TimeFrameHourTests (0 ms)
[ RUN      ] ISOModelFixture.TimeFrameDayOfMonthTests
[       OK ] ISOModelFixture.TimeFrameDayOfMonthTests (0 ms)
[ RUN      ] ISOModelFixture.TimeFrameDayOfWeekTests
[       OK ] ISOModelFixture.TimeFrameDayOfWeekTests (0 ms)
[ RUN      ] ISOModelFixture.TimeFrameMonthTests
[       OK ] ISOModelFixture.TimeFrameMonthTests (0 ms)
[ RUN      ] ISOModelFixture.TimeFrameTYDTests
[       OK ] ISOModelFixture.TimeFrameTYDTests (0 ms)
[ RUN      ] ISOModelFixture.UserModelInitializationTests
[       OK ] ISOModelFixture.UserModelInitializationTests (44 ms)
[ RUN      ] ISOModelFixture.UserModelDefaultsTests
[       OK ] ISOModelFixture.UserModelDefaultsTests (47 ms)
[ RUN      ] ISOModelFixture.UserModelOptionalPropertiesDefaultsTests
[       OK ] ISOModelFixture.UserModelOptionalPropertiesDefaultsTests (43 ms)
[ RUN      ] ISOModelFixture.UserModelOptionalPropertiesOverrideTests
[       OK ] ISOModelFixture.UserModelOptionalPropertiesOverrideTests (49 ms)
[----------] 14 tests from ISOModelFixture (490 ms total)

[----------] 4 tests from OptimizationCoverage
[ RUN      ] OptimizationCoverage.Constants_Values
[       OK ] OptimizationCoverage.Constants_Values (0 ms)
[ RUN      ] OptimizationCoverage.SolarRadiation_Math
[       OK ] OptimizationCoverage.SolarRadiation_Math (0 ms)
[ RUN      ] OptimizationCoverage.EpwData_Parsing
[       OK ] OptimizationCoverage.EpwData_Parsing (0 ms)
[ RUN      ] OptimizationCoverage.UserModel_ModernFeatures
[       OK ] OptimizationCoverage.UserModel_ModernFeatures (0 ms)
[----------] 4 tests from OptimizationCoverage (5 ms total)

[----------] Global test environment tear-down
[==========] 18 tests from 2 test suites ran. (500 ms total)
[  PASSED  ] 18 tests.
```
### Benchmark
```
c:\Git\ISOmodel\IsoModel\build_yaml_merged\Release>isomodel_benchmark.exe .
Loading test data from: .
Creating MonthlyModel
Creating HourlyModel
Benchmark: Running Simulations. Timing just the simulation. Iterations = 1000
Monthly simulation ran in 20.8507 us, average over 1000 loops.
Hourly simulation ran in 2463.71 us, average over 1000 loops.
Benchmark: Updating .ism properties with UserModel setters, creating simmodel, running monthly simulation.
Monthly simulation including modifying properties ran in 16.9825 us, average over 1000 loops.
Done!
```

### Standalone
```
c:\Git\ISOmodel\IsoModel\build_yaml_merged\Release>isomodel_standalone -m test_bldg.yaml
Monthly Results:
Month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW
1, 0, 0.01498967142, 2.641326323, 0.2578224281, 9.347674031, 0.8179086728, 2.187376303, 0, 0, 53.17564536, 0, 0, 0
2, 0, 0.02964839915, 2.385714098, 0.1996044604, 7.68602147, 0.6725163499, 1.975694725, 0, 0, 43.68897189, 0, 0, 0
3, 0, 0.08908422722, 2.641326323, 0.1841588772, 6.222024781, 0.5444186451, 2.187376303, 0, 0, 35.23914089, 0, 0, 0
4, 0, 0.2437607002, 2.556122248, 0.1782182683, 3.505628899, 0.3067377265, 2.116815777, 0, 0, 19.47334293, 0, 0, 0
5, 0, 0.8762071093, 2.641326323, 0.1473271018, 1.650345721, 0.1444029898, 2.187376303, 0, 0, 7.667990234, 0, 0, 0
6, 0, 1.730169589, 2.556122248, 0.1425746146, 0.7597045438, 0.0664731069, 2.116815777, 0, 0, 0.9169764944, 0, 0, 0
7, 0, 2.913090753, 2.641326323, 0.1473271018, 1.00786416, 0.08818673335, 2.187376303, 0, 0, 0, 0, 0, 0
8, 0, 1.519921992, 2.641326323, 0.1473271018, 0.622211602, 0.05444266283, 2.187376303, 0, 0, 0.5484206604, 0, 0, 0
9, 0, 0.6192120399, 2.556122248, 0.1782182683, 0.9997763022, 0.08747905689, 2.116815777, 0, 0, 4.471158628, 0, 0, 0
10, 0, 0.1256171721, 2.641326323, 0.2025747649, 3.540880475, 0.3098221911, 2.187376303, 0, 0, 19.90664085, 0, 0, 0
11, 0, 0.02568934654, 2.556122248, 0.2316837487, 6.019138126, 0.5266663407, 2.116815777, 0, 0, 34.20918865, 0, 0, 0
12, 0, 0.01236677725, 2.641326323, 0.2578224281, 8.696656427, 0.7609455242, 2.187376303, 0, 0, 49.47534341, 0, 0, 0

c:\Git\ISOmodel\IsoModel\build_yaml_merged\Release>isomodel_standalone -h test_bldg.yaml
Hourly results by month:
month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW
1, 0, 0, 2.74978, 0.257822, 7.28522, 0.186, 2.24088, 0, 0, 41.4661, 0, 0, 0
2, 0, 0, 2.48852, 0.207327, 5.80074, 0.168, 2.02735, 0, 0, 33.0167, 0, 0, 0
3, 0, 0, 2.78731, 0.210892, 4.22011, 0.1765, 2.26671, 0, 0, 23.961, 0, 0, 0
4, 0, 0.802112, 2.61266, 0.198416, 2.83319, 0.15375, 2.13526, 0, 0, 14.5286, 0, 0, 0
5, 0, 1.54334, 2.78731, 0.159208, 1.5605, 0.138, 2.26671, 0, 0, 5.81642, 0, 0, 0
6, 0, 3.21281, 2.68771, 0.142575, 1.32756, 0.12175, 2.18692, 0, 0, 1.42973, 0, 0, 0
7, 0, 4.64805, 2.71226, 0.148515, 1.59498, 0.119, 2.21505, 0, 0, 0.28226, 0, 0, 0
8, 0, 2.73221, 2.78731, 0.178218, 1.05553, 0.11125, 2.26671, 0, 0, 0.768581, 0, 0, 0
9, 0, 1.27468, 2.65019, 0.189505, 1.02726, 0.117, 2.16109, 0, 0, 3.25967, 0, 0, 0
10, 0, 0.0630526, 2.74978, 0.226931, 2.23488, 0.16025, 2.24088, 0, 0, 12.4503, 0, 0, 0
11, 0, 0, 2.68771, 0.236436, 4.27585, 0.17275, 2.18692, 0, 0, 24.2914, 0, 0, 0
12, 0, 0, 2.71226, 0.258416, 7.02635, 0.186, 2.21505, 0, 0, 39.9926, 0, 0, 0

c:\Git\ISOmodel\IsoModel\build_yaml_merged\Release>isomodel_standalone -h test_bldg_schedules.yaml
Hourly results by month:
month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW
1, 0, 0, 3.002, 0.0130693, 16.6916, 0.186, 3.196, 0.0246029, 0, 46.1865, 0, 0, 0
2, 0, 0, 2.716, 0.000594061, 13.4495, 0.168, 2.888, 0.0221614, 0, 37.1344, 0, 0, 0
3, 0, 0.00908381, 3.037, 0, 10.5838, 0.17825, 3.206, 0.0241333, 0, 28.1998, 0, 0, 0
4, 0, 0.982583, 2.86, 0, 7.40755, 0.1685, 3.08, 0.0244151, 0, 18.437, 0, 0, 0
5, 0, 2.00843, 3.037, 0, 4.67004, 0.168, 3.206, 0.0241333, 0, 9.73886, 0, 0, 0
6, 0, 3.76702, 2.93, 0, 2.90597, 0.177, 3.1, 0.023476, 0, 4.04501, 0, 0, 0
7, 0, 5.19478, 2.967, 0, 2.54555, 0.18475, 3.186, 0.0250724, 0, 2.01997, 0, 0, 0
8, 0, 3.35538, 3.037, 0, 2.53845, 0.18225, 3.206, 0.0241333, 0, 3.47563, 0, 0, 0
9, 0, 1.74363, 2.895, 0, 3.3997, 0.1645, 3.09, 0.0239455, 0, 6.85617, 0, 0, 0
10, 0, 0.146357, 3.002, 0.00653467, 6.73004, 0.1605, 3.196, 0.0246029, 0, 16.6229, 0, 0, 0
11, 0, 0.00738007, 2.93, 0.0130693, 10.5665, 0.17175, 3.1, 0.023476, 0, 28.4171, 0, 0, 0
12, 0, 0, 2.967, 0.0124753, 16.2766, 0.186, 3.186, 0.0250724, 0, 44.7369, 0, 0, 0

c:\Git\ISOmodel\IsoModel\build_yaml_merged\Release>
```



