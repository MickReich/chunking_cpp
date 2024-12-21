cmake_minimum_required(VERSION 3.10)
project(ChunkProcessor)

# Create documentation directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/docs/html)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTEST_INCLUDE_DIRS})

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

# Configure test output
set(GTEST_OUTPUT_DIR ${CMAKE_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${GTEST_OUTPUT_DIR})

# Ensure test results directory exists
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/results)

# Configure GTest
include(GoogleTest)
enable_testing()

# Add options for coverage and sanitizers
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fprofile-arcs -ftest-coverage --coverage -fexceptions")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    
    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)
    
    set(COVERAGE_EXCLUDES
        '/usr/*'
        '${PROJECT_SOURCE_DIR}/tests/*'
    )
    
    add_custom_target(coverage
        DEPENDS run_tests
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
        COMMAND ${LCOV} --directory . --zerocounters
        COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${CMAKE_BINARY_DIR}/coverage/coverage-%p.profraw
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target run_tests
        COMMAND ${LCOV} --directory . --capture --output-file coverage.info --ignore-errors empty,mismatch,range,unused
        COMMAND ${LCOV} --remove coverage.info ${COVERAGE_EXCLUDES} --output-file filtered.info --ignore-errors unused
        COMMAND ${GENHTML} filtered.info --output-directory ${CMAKE_BINARY_DIR}/coverage
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report"
    )
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
    tests/utils_test.cpp
    tests/chunk_strategies_test.cpp
    tests/chunk_compression_test.cpp
    tests/parallel_chunk_test.cpp
    tests/advanced_structures_test.cpp
    tests/advanced_chunk_strategies_test.cpp
    tests/sub_chunk_strategies_test.cpp
)

# Set the output directory for run_tests executable
set_target_properties(run_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Link test executable with GTest
target_link_libraries(run_tests
    chunk_processor
    ${GTEST_LIBRARIES}
    pthread
)

# Discover tests automatically
gtest_discover_tests(run_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    PROPERTIES
        ENVIRONMENT "GTEST_COLOR=1"
        FAIL_REGULAR_EXPRESSION ".*[Ff]ail.*"
)

# Create tests directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Generate test runner script
file(WRITE ${CMAKE_BINARY_DIR}/tests/run_tests.sh
"#!/bin/bash

# Create directory for test results if it doesn't exist
mkdir -p \"\${0%/*}\"
mkdir -p \"\${0%/*}/results\"

# Run the tests and generate XML report
cd \"\${0%/*}\" && ./run_tests --gtest_output=xml:test_results.xml --gtest_break_on_failure

# Check if tests passed
if [ $? -eq 0 ]; then
    echo 'All tests passed!'
    exit 0
else
    echo 'Some tests failed!'
    if [ -f test_results.xml ]; then
        cat test_results.xml
    else
        echo "Error: test_results.xml not found"
    fi
    exit 1
fi
")

# Make the test runner script executable
file(COPY ${CMAKE_BINARY_DIR}/tests/run_tests.sh
    DESTINATION ${CMAKE_BINARY_DIR}/tests
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE OWNER_EXECUTE
)

# Ensure test output directory exists
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

# Add test to CTest
add_test(
    NAME AllTests
    COMMAND ${CMAKE_BINARY_DIR}/tests/run_tests --gtest_output=xml:${CMAKE_BINARY_DIR}/tests/test_results.xml
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Set test properties
set_tests_properties(AllTests PROPERTIES
    ENVIRONMENT "GTEST_COLOR=1"
    FAIL_REGULAR_EXPRESSION ".*[Ff]ail.*"
    FIXTURES_REQUIRED test_output
)

# Add fixture to ensure test output directory exists
add_test(NAME create_test_output
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/tests
)
set_tests_properties(create_test_output PROPERTIES
    FIXTURES_SETUP test_output
) 