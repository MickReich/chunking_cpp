#!/bin/bash

# Build and run all tests
make clean
make test

# Run specific test suites if requested
if [ $# -gt 0 ]; then
    for suite in "$@"; do
        echo "Running test suite: $suite"
        make test-$suite
    done
fi 