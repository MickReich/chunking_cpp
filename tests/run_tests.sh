#!/bin/bash

# Create directory for test results if it doesn't exist
mkdir -p "${0%/*}"

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
