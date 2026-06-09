<!-- Last edited 05-Jan-2026 -->

The isomodel code uses CMake for compilation. 

# Instructions for installing and compiling within Linux (Tested with Ubuntu 24.04 LTS direct install and also on a WSL based on Ubuntu 24.04 LTS)

### 1) Install Development environment work
Make sure you have a full development environment, including gcc and cmake installed
```
sudo apt update
sudo apt install build-essential
sudo apt install cmake  

```
*Note: while gdb is not required, many find it very useful for debugging during active development so optionally install gdb*
```
sudo apt install gdb
```

### 2) Download the latest version of the ISOMODEL from github

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

### 3) Compile the code

At this point, if you want to change to a different branch (from main) for compiling, go into the repository and change the branch.  Then,

```
mkdir bin
cd bin
cmake ../src
```

This should run cmake and generate cmake files.

Now you should be able to just run make and compile everything.  Make will download dependencies (yaml and google test) on the fly. 

```
make
```

When complete you should have several files including ```isomodel_standalone```, ```isomodel_unit_tests```, ```isomodel_benchmark```, ```libisomodel.so```, and ```solar_debug```, ```test_bldg.yaml```, ```test.epw```,  and a new ```test_data``` subdirectory 

### 4) Test the Code
You can test the compilation with the command
```
./isomodel_unit_tests
```
which should run and give passed.  The full output looks like:
```
username:~/git/ISOmodel/IsoModel/bin$ ./isomodel_unit_tests
[==========] Running 13 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 13 tests from ISOModelFixture
[ RUN      ] ISOModelFixture.HourlyModelTests
[       OK ] ISOModelFixture.HourlyModelTests (41 ms)
[ RUN      ] ISOModelFixture.MonthlyModelTests
[       OK ] ISOModelFixture.MonthlyModelTests (22 ms)
[ RUN      ] ISOModelFixture.SunPositionAndRadiationTests
[       OK ] ISOModelFixture.SunPositionAndRadiationTests (37 ms)
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
[       OK ] ISOModelFixture.UserModelInitializationTests (20 ms)
[ RUN      ] ISOModelFixture.UserModelDefaultsTests
[       OK ] ISOModelFixture.UserModelDefaultsTests (21 ms)
[ RUN      ] ISOModelFixture.UserModelOptionalPropertiesDefaultsTests
[       OK ] ISOModelFixture.UserModelOptionalPropertiesDefaultsTests (20 ms)
[ RUN      ] ISOModelFixture.UserModelOptionalPropertiesOverrideTests
[       OK ] ISOModelFixture.UserModelOptionalPropertiesOverrideTests (23 ms)
[----------] 13 tests from ISOModelFixture (189 ms total)

[----------] Global test environment tear-down
[==========] 13 tests from 1 test suite ran. (189 ms total)
[  PASSED  ] 13 tests.
```

you can test the standalone code through the benchmark or running the standalone version directly 

To run the benchmark:
```
cd test_data
./isomodel_benchmark .
```

The output should be something like:
```
username:~/git/ISOmodel/IsoModel/bin$ ./isomodel_benchmark .
Loading test data from: .
Creating MonthlyModel
Creating HourlyModel
Benchmark: Running Simulations. Timing just the simulation. Iterations = 1000
Monthly simulation ran in 82.0691 us, average over 1000 loops.
Hourly simulation ran in 1647.67 us, average over 1000 loops.
Benchmark: Updating .ism properties with UserModel setters, creating simmodel, running monthly simulation.
Monthly simulation including modifying properties ran in 49.8753 us, average over 1000 loops.
Done!

```

To run the standalone code itself:

```
./isomodel_standalone -h test_bldg.yaml
```



The output should be something like:

```
username:~/git/ISOmodel/IsoModel/bin$ ./isomodel_standalone -h test_bldg.yaml
Hourly results by month:
month,ElecHeat,ElecCool,ElecIntLights,ElecExtLights,ElecFans,ElecPump,ElecEquipInt,ElecEquipExt,ElectDHW,GasHeat,GasCool,GasEquip,GasDHW
1, 0, 0, 2.74978, 0.257822, 7.28523, 0.186, 2.24088, 0, 0, 41.4661, 0, 0, 0
2, 0, 0, 2.48852, 0.207327, 5.80074, 0.168, 2.02735, 0, 0, 33.0167, 0, 0, 0
3, 0, 0, 2.78731, 0.210892, 4.22011, 0.1765, 2.26671, 0, 0, 23.961, 0, 0, 0
4, 0, 0.802112, 2.61266, 0.198416, 2.83319, 0.15375, 2.13526, 0, 0, 14.5286, 0, 0, 0
5, 0, 1.54334, 2.78731, 0.159208, 1.5605, 0.138, 2.26671, 0, 0, 5.81642, 0, 0, 0
6, 0, 3.21281, 2.68771, 0.142575, 1.32756, 0.12175, 2.18692, 0, 0, 1.42973, 0, 0, 0
7, 0, 4.64805, 2.71226, 0.148515, 1.59498, 0.119, 2.21505, 0, 0, 0.28226, 0, 0, 0
8, 0, 2.73221, 2.78731, 0.178218, 1.05554, 0.11125, 2.26671, 0, 0, 0.768581, 0, 0, 0
9, 0, 1.27468, 2.65019, 0.189505, 1.02726, 0.117, 2.16109, 0, 0, 3.25967, 0, 0, 0
10, 0, 0.0630526, 2.74978, 0.226931, 2.23488, 0.16025, 2.24088, 0, 0, 12.4503, 0, 0, 0
11, 0, 0, 2.68771, 0.236436, 4.27586, 0.17275, 2.18692, 0, 0, 24.2914, 0, 0, 0
12, 0, 0, 2.71226, 0.258416, 7.02635, 0.186, 2.21505, 0, 0, 39.9926, 0, 0, 0
(base) rmuehleisen@CSI356470:~/git/ISOmodel/IsoModel/bin$
```

Congratulations!  You just successfully compiled the ISOmodel!

