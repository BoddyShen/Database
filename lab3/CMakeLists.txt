cmake_minimum_required(VERSION 3.10)

# Set the project name
project(lab3)

# Specify the C++ standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_definitions (-O3 -Wall -Iinclude)

# Enable testing so that CTest knows about our tests.
enable_testing()

# Include directories
include_directories(include)

# Add source files from the directories
file(GLOB_RECURSE SOURCES src/*.cpp)

# Remove the one file that has the app’s main()
list(REMOVE_ITEM SOURCES
     ${CMAKE_SOURCE_DIR}/src/StartEngine.cpp
)

# Glob test files from test/unit/
file(GLOB_RECURSE TEST_SOURCES test/unit/*.cpp)

# For each test file, create a separate executable and register it with CTest.
foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src} ${SOURCES})
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

add_custom_target(run_unit_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
)

######### Lab 2 ###########
# Add end2end test files
file(GLOB_RECURSE TEST_SOURCES test/test_lab2_end2end.cpp)

# Create an executable for end2end tests
add_executable(test_lab2_end2end ${TEST_SOURCES} ${SOURCES})

# Add test_end2end_small_dataset test files
file(GLOB_RECURSE TEST_SOURCES test/test_lab2_end2end_small_dataset.cpp)

# Create an executable for test_end2end_small_dataset tests
add_executable(test_lab2_end2end_small_dataset ${TEST_SOURCES} ${SOURCES})


######### Lab 3 ###########
add_executable(db_engine
  src/StartEngine.cpp
  ${SOURCES}
)
