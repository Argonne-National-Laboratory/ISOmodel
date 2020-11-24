# This file lists and installs the Conan packages needed

set(CMAKE_CONAN_EXPECTED_HASH 773399d30bb924959b86883f95d64df6)
set(CMAKE_CONAN_VERSION "v0.15")

if(EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  file(MD5 "${CMAKE_BINARY_DIR}/conan.cmake" CMAKE_CONAN_HASH)
endif()

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake" OR NOT "${CMAKE_CONAN_HASH}" MATCHES "${CMAKE_CONAN_EXPECTED_HASH}")
  message(STATUS "Downloading conan.cmake ${CMAKE_CONAN_VERSION} from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/${CMAKE_CONAN_VERSION}/conan.cmake"
     "${CMAKE_BINARY_DIR}/conan.cmake")
else()
  message(STATUS "using existing conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_check(VERSION 1.21.0 REQUIRED)

message(STATUS "RUNNING CONAN")

# Add NREL remote and place it first in line, since we vendored dependencies to NREL's repo, they will be picked first
conan_add_remote(NAME nrel INDEX 0
  URL https://api.bintray.com/conan/commercialbuilding/nrel)

conan_add_remote(NAME bincrafters
  URL https://api.bintray.com/conan/bincrafters/public-conan)

list(APPEND CONAN_BUILD "outdated")

if(BUILD_RUBY_BINDINGS)
  set(CONAN_RUBY "openstudio_ruby/2.5.5@nrel/stable#29449dcdcc813fb3f4730365902afc3c")
endif()

# This will create the conanbuildinfo.cmake in the current binary dir, not the cmake_binary_dir
conan_cmake_run(REQUIRES
  boost/1.71.0
  gtest/1.10.0
  swig/4.0.2
  ${CONAN_RUBY}

  BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
  OPTIONS ${CONAN_OPTIONS}
  BUILD ${CONAN_BUILD}
  # Passes `-u, --update`    to conan install: Check updates exist from upstream remotes
  # That and build=outdated should ensure we track the right
  UPDATE
)

set(CONAN_ALREADY_RUN TRUE)

message(STATUS "DONE RUNNING CONAN")


