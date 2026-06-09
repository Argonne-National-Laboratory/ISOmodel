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
mkdir bin
cd bin
```

Now, if you are lucky, cmake will autodetect Visual Studio 26 and you can create the Visual Studio 2026 project files with the command line:
 
```
cmake ../src
```

If all goes well things will run, print some output, a couple warnings but then finally end with the following lines (times will differ on your machine)

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
or 
```
cmake --build bin --config Release -- /v:m
```

The compiled code (isomodel.dll, isomodel.lib isomodel_benchmark.exe isomodel_standalone.exe, isomodel_unit_tests.exe, etc) is in the build/Release directory.

