The isomodel code uses CMake for compilation. 

# Instructions for installing and compiling within WSL (Tested with WSL based on Ubuntu 24.04 LTS)

### 1) Install Development environment work
Make sure you have a full development environment, including gcc and gdb installed
```
sudo apt update
sudo apt install build-essential
sudo apt install cmake  

```
*Note: while gdb is not required, many find it very useful for debugging during active environment so optionally install gdb*
```
sudo apt install gdb
```

### 2) Install Boost Libraries
The following will install the precompiled versions of the latest stable boost libraries in locations where cmake can find them automagically.
```
sudo apt install libboost-all-dev
```
This installs ```Boost-1.83.0``` on Ubuntu LTS 24.04.3 on 06-oct-2025

### 3) Download the latest version of the ISOMODEL from github

if you don't already have one, create a home directory for your git repositories
```
mkdir ~/git
```
now, go to the main git directory and use git to clone the repository.  This will automagically create an ```git/ISOmodel``` subdirectory.  After that, go to the main source directory ```git/ISOmodel/IsoModel```


```
cd ~/git
git clone https://github.com/Argonne-National-Laboratory/ISOmodel.git
cd ISOmodel/IsoModel
```

### 4) Compiling the code

At this point, if you want to change to a different branch (from main) for compiling, go into the repository and change the branch

```
mkdir bin
cd bin
cmake ../src
```

This should run cmake and find both Boost and Gtest without having to give them on the command line.

Now you should be able to just run make and compile everything 

```
make
```

*Note: on Ubuntu WSL 24.04 installed as above we will get several warndings about struct unary_function.  This is okay - it doesnt stop the code from finishing compiling*

When complete you should have several files including ```isomodel_standalone```, ```isomodel_unit_tests```, ```isomodel_benchmark```, ```libisomodel.so```, and ```solar_debug```, and a new ```test_data``` subdirectory

### 5) Testing the Code
You can test the compilation with the command
```
isomodel_unit_tests
```
which should run and give passed.  The full output looks like:
```
(base) rmuehleisen@CSI356470:~/git/ISOmodel/IsoModel/bin$ ./isomodel_unit_tests
[==========] Running 15 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 15 tests from ISOModelFixture
[ RUN      ] ISOModelFixture.HourlyModelTests
[       OK ] ISOModelFixture.HourlyModelTests (440 ms)
[ RUN      ] ISOModelFixture.MonthlyModelTests
[       OK ] ISOModelFixture.MonthlyModelTests (58 ms)
[ RUN      ] ISOModelFixture.PropsKeyValueTests
[       OK ] ISOModelFixture.PropsKeyValueTests (0 ms)
[ RUN      ] ISOModelFixture.PropsMissingValueTests
[       OK ] ISOModelFixture.PropsMissingValueTests (2 ms)
[ RUN      ] ISOModelFixture.SunPositionAndRadiationTests
[       OK ] ISOModelFixture.SunPositionAndRadiationTests (111 ms)
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
[       OK ] ISOModelFixture.UserModelInitializationTests (58 ms)
[ RUN      ] ISOModelFixture.UserModelDefaultsTests
[       OK ] ISOModelFixture.UserModelDefaultsTests (67 ms)
[ RUN      ] ISOModelFixture.UserModelOptionalPropertiesDefaultsTests
[       OK ] ISOModelFixture.UserModelOptionalPropertiesDefaultsTests (176 ms)
[ RUN      ] ISOModelFixture.UserModelOptionalPropertiesOverrideTests
[       OK ] ISOModelFixture.UserModelOptionalPropertiesOverrideTests (121 ms)
[----------] 15 tests from ISOModelFixture (1037 ms total)

[----------] Global test environment tear-down
[==========] 15 tests from 1 test suite ran. (1037 ms total)
[  PASSED  ] 15 tests.
```

you can also test the standalone code by running a file in the test_data directory

```
cd test_data
../isomodel_standalone -h -i SmallOffice_v2.ism
```

Or, if you are using the updated yaml version in the yaml_update branch, you would give the command

```
cd test_data
../isomodel_standalone -h -i SmallOffice_v2_ism.yaml
```

The output should be something like:

```
(base) rmuehleisen@CSI356470:~/git/ISOmodel/IsoModel/bin/test_data$ ../isomodel_standalone -h SmallOffice_v2_ism.yaml
Hourly results by month:
month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW
1, 0, 0, 2.74978, 0.257822, 7.25748, 0.186, 2.24088, 0, 0, 41.3248, 0, 0, 0
2, 0, 0, 2.48852, 0.207327, 5.78027, 0.168, 2.02735, 0, 0, 32.9002, 0, 0, 0
3, 0, 0, 2.78731, 0.210892, 4.20455, 0.1765, 2.26671, 0, 0, 23.8715, 0, 0, 0
4, 0, 0.803466, 2.61266, 0.198416, 2.82539, 0.15375, 2.13526, 0, 0, 14.4764, 0, 0, 0
5, 0, 1.5465, 2.78731, 0.159208, 1.56159, 0.138, 2.26671, 0, 0, 5.79526, 0, 0, 0
6, 0, 3.21538, 2.68771, 0.142575, 1.3296, 0.12175, 2.18692, 0, 0, 1.42311, 0, 0, 0
7, 0, 4.64883, 2.71226, 0.148515, 1.59728, 0.119, 2.21505, 0, 0, 0.280847, 0, 0, 0
8, 0, 2.73518, 2.78731, 0.178218, 1.0586, 0.11075, 2.26671, 0, 0, 0.765168, 0, 0, 0
9, 0, 1.27682, 2.65019, 0.189505, 1.02824, 0.11675, 2.16109, 0, 0, 3.24793, 0, 0, 0
10, 0, 0.0637051, 2.74978, 0.226931, 2.22808, 0.16, 2.24088, 0, 0, 12.406, 0, 0, 0
11, 0, 0, 2.68771, 0.236436, 4.26135, 0.17275, 2.18692, 0, 0, 24.208, 0, 0, 0
12, 0, 0, 2.71226, 0.258416, 7.00312, 0.186, 2.21505, 0, 0, 39.8604, 0, 0, 0
(base) rmuehleisen@CSI356470:~/git/ISOmodel/IsoModel/bin/test_data$
```

fsdf

