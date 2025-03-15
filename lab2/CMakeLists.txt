cmake_minimum_required(VERSION 3.10)

# Set the project name
project(lab2)

# Specify the C++ standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions (-Wall -Iinclude)

# Include directories
include_directories(include)

# Add source files from the directories
file(GLOB_RECURSE SOURCES src/*.cpp)

# Define the executable
# add_executable(main main.cpp ${SOURCES})
