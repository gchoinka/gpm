#
#  Copyright 2018 Gerard Choinka
#  
#  Distributed under the Boost Software License, Version 1.0.
#  (See accompanying file LICENSE_1_0.txt or
#  copy at http://www.boost.org/LICENSE_1_0.txt)
#

cmake_minimum_required ( VERSION 3.9.2 )

include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.17.tar.gz"
    SHA1 "a438452aa14fc5cbd58f080eee45f07351b27d2f"
)

project (gpm)

set(BOOST_MIN_VERSION "1.68.0")
hunter_add_package(Boost COMPONENTS system program_options)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS system program_options)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(examples)

