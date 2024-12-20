#!/bin/bash

# Create directory for test results if it doesn't exist
mkdir -p "${0%/*}"

# Run the tests and generate XML report
./run_tests --gtest_output=xml:/mnt/c/Users/jonat/OneDrive/Documents/Python Scripts/chunking_cpp/tests/test_results.xml --gtest_break_on_failure

# Check if tests passed
if [ $? -eq 0 ]; then
    echo 'All tests passed!'
    exit 0
else
    echo 'Some tests failed!'
    cat /mnt/c/Users/jonat/OneDrive/Documents/Python Scripts/chunking_cpp/tests/test_results.xml
    exit 1
fi
