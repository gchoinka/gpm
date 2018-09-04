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

project(gpm CXX)


option(GPM_BUILD_TESTS "Enable to build the tests" ON)
option(GPM_CXX_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF)


include("cmake/CompilerSettings.cmake")

hunter_add_package(Boost)
set(BOOST_MIN_VERSION "1.68.0")
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)

hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)

if (${GPM_BUILD_TESTS})
    hunter_add_package(Catch)
    find_package(Catch2 CONFIG REQUIRED)
    enable_testing()
endif()

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(examples)
add_subdirectory(tests)
