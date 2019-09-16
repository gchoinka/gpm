#
#  Copyright 2018 Gerard Choinka
#  
#  Distributed under the Boost Software License, Version 1.0.
#  (See accompanying file LICENSE_1_0.txt or
#  copy at http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.12.1)

project(gpm VERSION 0.1.0 LANGUAGES CXX)


include("pmm.cmake")
pmm(VCPKG REVISION 2019.08 REQUIRES fmt spdlog boost)

include("cmake/CompilerSettings.cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(GPM_BUILD_TESTS "Enable to build the tests" ON)
option(GPM_BUILD_EXAMPLES "Enable to build the tests" ON) # TODO split EXAMPLES and BENCHMARK
option(GPM_BUILD_BENCHMARK "Enable to build benchmarks" ON)
option(GPM_BUILD_SKETCH "Enable to build sketch" OFF)
option(GPM_CXX_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)
option(GPM_BUILD_WPROFILER "Enable to build with profiler (search for depandencies in system libgoogle-perftools-dev)" OFF)


# program_options fmt spdlog catch2 benchmark


set(BOOST_MIN_VERSION "1.68.0") 
find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS program_options REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)


if(${GPM_BUILD_TESTS})
    find_package(Catch2 CONFIG REQUIRED)
    enable_testing()
endif()

if(${GPM_BUILD_BENCHMARK})
    find_package(benchmark CONFIG REQUIRED)
endif()

if(${GPM_BUILD_WPROFILER})
  find_package(Gperftools REQUIRED)
endif()

find_package(ClangFormat)

find_package(Threads)

add_library(Outcome INTERFACE)
target_include_directories(Outcome SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ned14_outcome/include)

add_library(Gpm INTERFACE)
target_include_directories(Gpm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Gpm INTERFACE Boost::boost)

add_library(Frozen INTERFACE)
target_include_directories(Frozen INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/frozen/include)


if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()

if(${GPM_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()

if(${GPM_BUILD_SKETCH})
    add_subdirectory(sketch)
endif()
