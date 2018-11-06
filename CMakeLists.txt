#
#  Copyright 2018 Gerard Choinka
#  
#  Distributed under the Boost Software License, Version 1.0.
#  (See accompanying file LICENSE_1_0.txt or
#  copy at http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.12.1)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.44.tar.gz"
    SHA1 "c4cfcc0cd39fdae8de08f6205b7f34cab4a7ba79"
)

project(gpm VERSION 0.1.0 LANGUAGES CXX)

include("cmake/CompilerSettings.cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(GPM_BUILD_TESTS "Enable to build the tests" ON)
option(GPM_BUILD_EXAMPLES "Enable to build the tests" ON) # TODO split EXAMPLES and BENCHMARK
option(GPM_BUILD_BENCHMARK "Enable to build benchmarks" ON)
option(GPM_BUILD_SKETCH "Enable to build sketch" OFF)
option(GPM_CXX_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)



hunter_add_package(Boost COMPONENTS program_options)
set(BOOST_MIN_VERSION "1.68.0") 
find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS program_options REQUIRED)

hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)

hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)


if(${GPM_BUILD_TESTS})
    hunter_add_package(Catch)
    find_package(Catch2 CONFIG REQUIRED)
    enable_testing()
endif()

if(${GPM_BUILD_BENCHMARK})
    hunter_add_package(benchmark)
    find_package(benchmark CONFIG REQUIRED)
endif()

find_package(ClangFormat)

find_package(Threads)

add_library(Outcome INTERFACE)
target_include_directories(Outcome SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ned14_outcome/include)

add_library(Dyno INTERFACE)
target_include_directories(Dyno SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ldionne_dyno/include)

add_library(Gpm INTERFACE)
target_include_directories(Gpm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)



if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()

if(${GPM_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()

if(${GPM_BUILD_SKETCH})
    add_subdirectory(sketch)
endif()
