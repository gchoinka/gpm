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
    URL "https://github.com/ruslo/hunter/archive/v0.23.22.tar.gz"
    SHA1 "dffc6269efd60e79ea2e28a927cce94d3e4a6e03"
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


hunter_add_package(Boost)
set(BOOST_MIN_VERSION "1.66.0") # hunter on windows does not have 1.68
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)

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

find_program(CLANG_FORMAT NAMES "clang-format" "clang-format-9.0" "clang-format-8.0" "clang-format-7.0" "clang-format-6.0" "clang-format-5.0" "clang-format-4.0" "clang-format-3.0" DOC "Path to clang-format executable")

if(NOT CLANG_FORMAT)
  message(STATUS "clang-format not found.")
else()
  message(STATUS "clang-format found: ${CLANG_FORMAT}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include) # remove this global

# #remove this 
# include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
# ExternalProject_Add(frozen
#     PREFIX ${CMAKE_CURRENT_BINARY_DIR}/frozen
#     GIT_REPOSITORY https://github.com/serge-sans-paille/frozen.git
#     GIT_TAG        master
#     CMAKE_ARGS -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
#     UPDATE_COMMAND ""
#     CONFIGURE_COMMAND ""
#     BUILD_COMMAND ""
#     INSTALL_COMMAND ""
# )

include_directories(${CMAKE_CURRENT_BINARY_DIR}/frozen/include/)


if(${GPM_BUILD_TESTS})
    add_subdirectory(tests)
endif()

if(${GPM_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()


if(${GPM_BUILD_SKETCH})
    add_subdirectory(sketch)
endif()    
