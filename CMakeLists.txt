cmake_minimum_required(VERSION 3.10)
project(ChunkProcessor VERSION 0.1.0)

# Create documentation directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/docs/html)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Try multiple methods to find pybind11
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Method 1: Try standard CMake find_package
set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND)
    # Method 2: Try pip installed location
    execute_process(
        COMMAND "${Python_EXECUTABLE}"
        -c "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE PYBIND11_FIND_RESULT
    )
    if(PYBIND11_FIND_RESULT EQUAL 0)
        list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")
        find_package(pybind11 CONFIG QUIET)
    endif()
endif()

if(NOT pybind11_FOUND)
    # Method 3: Try to fetch pybind11 using FetchContent
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11
        GIT_TAG        v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)
endif()

# Find Python and get site-packages directory
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c "import site; print(site.getsitepackages()[0])"
    OUTPUT_VARIABLE Python_SITEARCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add Python support
# enable_language(Python)

# Add compiler-specific flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

# Check for CUDA support
find_package(CUDA QUIET)
if(CUDA_FOUND)
    find_path(CUDA_RUNTIME_HEADER cuda_runtime.h
        HINTS ${CUDA_TOOLKIT_ROOT_DIR}/include
        PATH_SUFFIXES cuda
    )
    if(CUDA_RUNTIME_HEADER)
        message(STATUS "CUDA Runtime found - enabling GPU support")
        add_definitions(-DHAVE_CUDA)
        include_directories(${CUDA_RUNTIME_HEADER})
        
        # Add GPU chunking library only if CUDA is available
        add_library(gpu_chunking
            src/demo_gpu_chunking.cpp
        )
        
        target_include_directories(gpu_chunking PUBLIC
            ${PROJECT_SOURCE_DIR}/include
            ${CUDA_RUNTIME_HEADER}
        )
        
        target_link_libraries(gpu_chunking
            PRIVATE
            ${CUDA_LIBRARIES}
        )
        
        # Add GPU demo executable
        add_executable(gpu_chunking_demo src/demo_gpu_chunking.cpp)
        target_link_libraries(gpu_chunking_demo
            PRIVATE
            gpu_chunking
            ${CUDA_LIBRARIES}
        )
    else()
        message(STATUS "CUDA Runtime not found - disabling GPU support")
    endif()
else()
    message(STATUS "CUDA not found - disabling GPU support")
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${GTEST_INCLUDE_DIRS})

# Find Boost package with required components
find_package(Boost REQUIRED COMPONENTS graph)

# Add library target with all source files
add_library(chunk_processor
    src/main.cpp
    src/sophisticated_chunking_demo.cpp
    src/benchmark.cpp
    src/demo_neural_chunking.cpp
)

# Make headers available for the library
target_include_directories(chunk_processor PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Link Boost to the library
target_link_libraries(chunk_processor 
    PRIVATE 
    Boost::graph
    $<$<BOOL:${CUDA_FOUND}>:gpu_chunking>
)

# Add executable targets
add_executable(chunk_processor_exe src/main.cpp)
add_executable(benchmark_exe src/benchmark.cpp)
add_executable(neural_chunking_demo src/demo_neural_chunking.cpp)

# Link executables with the library
target_link_libraries(chunk_processor_exe chunk_processor)
target_link_libraries(benchmark_exe chunk_processor)
target_link_libraries(neural_chunking_demo chunk_processor)

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

# Add test executable with all test files
add_executable(run_tests
    tests/test_main.cpp
    tests/chunk_test.cpp
    tests/utils_test.cpp
    tests/chunk_strategies_test.cpp
    tests/chunk_compression_test.cpp
    tests/parallel_chunk_test.cpp
    tests/advanced_structures_test.cpp
    tests/advanced_chunk_strategies_test.cpp
    tests/chunking_methods_sophisticated_test.cpp
    tests/test_neuralnetwork.cpp  # Added new test file
)

# Set the output directory for run_tests executable
set_target_properties(run_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# Link test executable with GTest and Boost
target_link_libraries(run_tests
    PRIVATE
    chunk_processor
    sophisticated_chunking
    neural_chunking
    chunk_benchmark
    ${GTEST_LIBRARIES}
    pthread
    Boost::graph
)

# Configure test discovery
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

# Add the sophisticated chunking library
add_library(sophisticated_chunking
    src/sophisticated_chunking_demo.cpp
)

target_include_directories(sophisticated_chunking PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Add neural chunking library
add_library(neural_chunking
    src/demo_neural_chunking.cpp
)

target_include_directories(neural_chunking PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Link the specialized libraries to the main library
target_link_libraries(chunk_processor
    PRIVATE
    sophisticated_chunking
    neural_chunking
)

# Add benchmark library
add_library(chunk_benchmark
    src/benchmark.cpp
)

target_include_directories(chunk_benchmark PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(chunk_benchmark
    PRIVATE
    chunk_processor
)

# Add individual test executables
add_executable(chunking_methods_sophisticated_test
    tests/chunking_methods_sophisticated_test.cpp
)

add_executable(neural_network_test
    tests/test_neuralnetwork.cpp
)

# Link test executables
target_link_libraries(chunking_methods_sophisticated_test
    sophisticated_chunking
    gtest
    gtest_main
)

target_link_libraries(neural_network_test
    neural_chunking
    gtest
    gtest_main
)

# Add to test suite
add_test(NAME chunking_methods_sophisticated COMMAND chunking_methods_sophisticated_test)
add_test(NAME neural_network COMMAND neural_network_test)

# Install targets
install(TARGETS 
    chunk_processor
    sophisticated_chunking
    neural_chunking
    chunk_benchmark
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Find pybind11
find_package(pybind11 REQUIRED)

# Add Python module
pybind11_add_module(chunking_cpp 
    bindings/python/chunk_bindings.cpp
)

# Link with our libraries
target_link_libraries(chunking_cpp PRIVATE
    chunk_processor
    sophisticated_chunking
    neural_chunking
    chunk_benchmark
    $<$<BOOL:${nlohmann_json_FOUND}>:nlohmann_json::nlohmann_json>
)

# Set Python module properties
set_target_properties(chunking_cpp PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/chunking_cpp
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/chunking_cpp
)

# Install Python module
install(TARGETS chunking_cpp
    LIBRARY DESTINATION ${Python_SITEARCH}/chunking_cpp
    RUNTIME DESTINATION ${Python_SITEARCH}/chunking_cpp
)

# Create Python package structure
add_custom_command(TARGET chunking_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/python/chunking_cpp
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/python/chunking_cpp/__init__.py
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:chunking_cpp> ${CMAKE_BINARY_DIR}/python/chunking_cpp/
)

# Add Python tests directory
if(BUILD_TESTING)
    add_subdirectory(tests/python)
endif()

# Add dependencies for Python bindings
find_package(PostgreSQL QUIET)
find_package(libpqxx QUIET)
find_package(mongocxx QUIET)
find_package(RdKafka QUIET)
find_package(RabbitMQ QUIET)
find_package(nlohmann_json QUIET)

# Create build directory for generated headers
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)

# Configure optional features
configure_file(
    ${CMAKE_SOURCE_DIR}/include/config.hpp.in
    ${CMAKE_BINARY_DIR}/include/config.hpp
)

# Add generated config directory to include paths
include_directories(${CMAKE_BINARY_DIR}/include)

# Define compile definitions based on available packages
if(PostgreSQL_FOUND AND libpqxx_FOUND)
    target_compile_definitions(chunking_cpp PRIVATE HAVE_POSTGRESQL)
    target_compile_definitions(chunk_processor PRIVATE HAVE_POSTGRESQL)
    target_link_libraries(chunking_cpp PRIVATE 
        PostgreSQL::PostgreSQL
        libpqxx::pqxx
    )
else()
    message(STATUS "PostgreSQL support disabled - libpqxx not found")
endif()

if(mongocxx_FOUND)
    target_compile_definitions(chunking_cpp PRIVATE HAVE_MONGODB)
    target_compile_definitions(chunk_processor PRIVATE HAVE_MONGODB)
    target_link_libraries(chunking_cpp PRIVATE 
        mongocxx::mongocxx
    )
else()
    message(STATUS "MongoDB support disabled - mongocxx not found")
endif()

if(RdKafka_FOUND)
    target_compile_definitions(chunking_cpp PRIVATE HAVE_KAFKA)
    target_link_libraries(chunking_cpp PRIVATE 
        RdKafka::rdkafka++
    )
else()
    message(STATUS "Kafka support disabled - RdKafka not found")
endif()

if(RabbitMQ_FOUND)
    target_compile_definitions(chunking_cpp PRIVATE HAVE_RABBITMQ)
    target_link_libraries(chunking_cpp PRIVATE 
        RabbitMQ::rabbitmq
    )
endif()

if(nlohmann_json_FOUND)
    target_compile_definitions(chunking_cpp PRIVATE HAVE_JSON)
    target_link_libraries(chunking_cpp PRIVATE 
        nlohmann_json::nlohmann_json
    )
endif()

# Add include directories for Python module
target_include_directories(chunking_cpp PRIVATE
    ${PYTHON_INCLUDE_DIRS}
    ${pybind11_INCLUDE_DIRS}
)

# Force using system libstdc++
set_target_properties(chunking_cpp PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "/usr/lib/x86_64-linux-gnu"
)

# Check C++ compiler version and available features
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
        OUTPUT_VARIABLE GCC_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    if(GCC_VERSION VERSION_LESS 9.1)
        set(CMAKE_CXX_STANDARD 14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
        message(STATUS "Using C++14 due to older GCC version")
    else()
        set(CMAKE_CXX_STANDARD 17)
    endif()
endif()

# Add rpath settings for better library resolution
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(APPEND CMAKE_INSTALL_RPATH "/usr/lib/x86_64-linux-gnu")
  