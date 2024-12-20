#!/bin/bash

# Create directory for test results if it doesn't exist
mkdir -p "${0%/*}"

# Run the tests and generate XML report
"${0%/*}/run_tests" --gtest_output=xml:"${0%/*}/test_results.xml" --gtest_break_on_failure

# Check if tests passed
if [ $? -eq 0 ]; then
    echo 'All tests passed!'
    exit 0
else
    echo 'Some tests failed!'
    if [ -f "${0%/*}/test_results.xml" ]; then
        cat "${0%/*}/test_results.xml"
    else
        echo "Error: test_results.xml not found"
    fi
    exit 1
fi
