cmake_minimum_required(VERSION 3.10)
project(ChunkProcessor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add library target
add_library(chunk_processor
    src/main.cpp
    # Add other source files here
)

# Make headers available for the library
target_include_directories(chunk_processor PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Add executable target
add_executable(chunk_processor_exe src/main.cpp)
target_link_libraries(chunk_processor_exe chunk_processor)

# Enable testing
enable_testing()

# Add options for coverage and sanitizers
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

if(ENABLE_SANITIZERS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
endif()

# Find GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add test executable
add_executable(run_tests
    tests/test_main.cpp
    tests/chunk_test.cpp
    tests/chunk_strategies_test.cpp
    tests/chunk_compression_test.cpp
    tests/parallel_chunk_test.cpp
    tests/advanced_structures_test.cpp
)

# Link test executable with GTest
target_link_libraries(run_tests
    chunk_processor
    ${GTEST_LIBRARIES}
    pthread
)

# Create tests directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Generate test runner script
file(WRITE ${CMAKE_BINARY_DIR}/tests/run_tests.sh
"#!/bin/bash

# Create directory for test results if it doesn't exist
mkdir -p \"\${0%/*}\"

# Run the tests and generate XML report
./run_tests --gtest_output=xml:test_results.xml

# Check if tests passed
if [ $? -eq 0 ]; then
    echo 'All tests passed!'
    exit 0
else
    echo 'Some tests failed!'
    exit 1
fi
")

# Make the test runner script executable
file(COPY ${CMAKE_BINARY_DIR}/tests/run_tests.sh
    DESTINATION ${CMAKE_BINARY_DIR}/tests
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# Ensure test binary is copied to tests directory
add_custom_command(TARGET run_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:run_tests> ${CMAKE_BINARY_DIR}/tests/
)

# Create symbolic link for convenience
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/tests/run_tests.sh
    ${CMAKE_BINARY_DIR}/tests/run_tests
)

# Add test to CTest
add_test(NAME AllTests COMMAND ${CMAKE_BINARY_DIR}/tests/run_tests) 