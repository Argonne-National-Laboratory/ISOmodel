The isomodel code uses CMake for compilation. 

# Instructions for installing and compiling within WSL (Tested with WSL based on Ubuntu 24.04 LTS)

1) Install Development environment work
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

2) Install Boost Libraries
The following will install the precompiled versions of the latest stable boost libraries in locations where cmake can find them automagically.
```
sudo apt install libboost-all-dev
```
3) Install and compile Google Test
The following will download the latest googltest source and then the user can compile and install it to /usr/local/lib

```
sudo apt install libgtest-dev

cd /usr/src/googletest
sudo cmake .
sudo cmake --build . --target install
```

```
cd ~\git\googletest
~\git\googletest: mkdir build
~\git\googletest: cd build
~\git\googletest\build: cmake .. -DBUILD_GMOCK=OFF


now we install the google test libraries to the "standard" place so they are easily found

~\git\googletest\build: sudo make install

this will install stuff to /usr/local/incluge/gtest  and /usr/local/lib



This installs precompiled versions of the latest stable boost libraries and installs them in locations found by cmake without needing long command lines.
```

4) Download the latest version of the ISOMODEL from github

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

5) Compiling the code

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

*Note: on Ubuntu WSL 24.04 with the gtest libs installed as above we will get several warndings about struct unary_function.  This is okay - it doesnt stop the code from finishing compiling*

When complete you should have several files including ```isomodel_standalone```, ```isomodel_unit_tests```, ```isomodel_benchmark```, ```libisomodel.so```, and ```solar_debug```, and a new ```test_data``` subdirectory

You can test the compilation with the command
```
isomodel_unit_tests
```
which should run and give passed

you can also test the standalone code by running a file in the test_data directory

```
cd test_data
../isomodel_standalone -i SmallOffice_v2.ism
```

Or, if you are using the updated yaml version in the yaml_update branch, you would give the command

```
cd test_data
../isomodel_standalone -i SmallOffice_v2_ism.yaml
```


