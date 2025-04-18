cmake_minimum_required(VERSION 3.22)
project(gpm_hello_world VERSION 1.0.0 LANGUAGES CXX)

# Specify C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(include)

# Add executable
add_executable(gpm_hallo_world
    src/main.cpp
    src/gpm.cpp
)

# Install target
install(TARGETS gpm_hallo_world DESTINATION bin)
