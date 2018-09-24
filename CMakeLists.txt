#
#  Copyright 2018 Gerard Choinka
#  
#  Distributed under the Boost Software License, Version 1.0.
#  (See accompanying file LICENSE_1_0.txt or
#  copy at http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required(VERSION 3.9.2)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.27.tar.gz"
    SHA1 "70d75a25f895df3da00d8382417e8c2a08c93173"
)

project(gpm VERSION 0.1.0 LANGUAGES CXX)


option(GPM_BUILD_TESTS "Enable to build the tests" ON)
option(GPM_BUILD_EXAMPLES "Enable to build the tests" ON)
option(GPM_BUILD_BENCHMARK "Enable to build benchmarks" ON)
option(GPM_BUILD_SKETCH "Enable to build the sketch" OFF)
option(GPM_CXX_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)


include("cmake/CompilerSettings.cmake")

set(GeneratedIncludesDir "${CMAKE_BINARY_DIR}/generated_includes")
file(MAKE_DIRECTORY "${GeneratedIncludesDir}")
include_directories("${GeneratedIncludesDir}")


hunter_add_package(Boost COMPONENTS program_options)
set(BOOST_MIN_VERSION "1.68.0") 
find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS program_options REQUIRED)

hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)

if(${GPM_BUILD_TESTS})
    hunter_add_package(Catch)
    find_package(Catch2 CONFIG REQUIRED)
    enable_testing()
endif()

if(${GPM_BUILD_BENCHMARK})
    hunter_add_package(benchmark)
    find_package(benchmark CONFIG REQUIRED)
endif()

hunter_add_package(cxxopts)
find_package(cxxopts CONFIG REQUIRED)


find_program(CLANG_FORMAT NAMES clang-format clang-format-9 clang-format-9.0 clang-format-8 clang-format-8.0 clang-format-7 clang-format-7.0 clang-format-6 clang-format-6.0 clang-format-5 clang-format-5.0 clang-format-4 clang-format-4.0 clang-format-3 clang-format-3.0 DOC "Path to clang-format executable")

if(NOT CLANG_FORMAT)
  message(STATUS "clang-format not found.")
else()
  message(STATUS "clang-format found: ${CLANG_FORMAT}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include) # TODO replace this calls to include_directories

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ned14_outcome/include)

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ldionne_dyno/include)



if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()

if(${GPM_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()


if(${GPM_BUILD_SKETCH})
    add_subdirectory(sketch)
endif()    
