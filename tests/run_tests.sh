#!/bin/bash

# Create directory for test results if it doesn't exist
mkdir -p "${0%/*}"
mkdir -p "${0%/*}/results"

# Run the tests and generate XML report
cd "${0%/*}" && ./run_tests --gtest_output=xml:test_results.xml --gtest_break_on_failure

# Check if tests passed
if [ $? -eq 0 ]; then
    echo 'All tests passed!'
    exit 0
else
    echo 'Some tests failed!'
    if [ -f test_results.xml ]; then
        cat test_results.xml
    else
        echo Error:test_results.xmlnotfound
    fi
    exit 1
fi
