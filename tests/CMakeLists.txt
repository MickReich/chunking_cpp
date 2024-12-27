# Find all test files automatically
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Create test executable
add_executable(run_tests ${TEST_SOURCES})

# Link against gtest and project libraries
target_link_libraries(run_tests
    PRIVATE
    gtest
    gtest_main
    chunking_lib
)