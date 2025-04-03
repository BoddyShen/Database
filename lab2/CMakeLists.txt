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
add_executable(test_unit test/test_unit.cpp ${SOURCES})

# Add end2end test files
file(GLOB_RECURSE TEST_SOURCES test/test_end2end.cpp)

# Create an executable for end2end tests
add_executable(test_end2end ${TEST_SOURCES} ${SOURCES})

# Add test_end2end_small_dataset test files
file(GLOB_RECURSE TEST_SOURCES test/test_end2end_small_dataset.cpp)

# Create an executable for test_end2end_small_dataset tests
add_executable(test_end2end_small_dataset ${TEST_SOURCES} ${SOURCES})
