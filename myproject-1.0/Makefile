# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I.

# Detect OS for GTest configuration
ifeq ($(OS),Windows_NT)
    GTEST_DIR = $(VCPKG_ROOT)/installed/x64-windows
    GTEST_INCLUDE = -I$(GTEST_DIR)/include
    GTEST_LIBS = -L$(GTEST_DIR)/lib -lgtest -lgtest_main
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        GTEST_INCLUDE = -I/usr/include
        GTEST_LIBS = -lgtest -lgtest_main -pthread
    endif
    ifeq ($(UNAME_S),Darwin)
        GTEST_INCLUDE = -I/usr/local/include
        GTEST_LIBS = -L/usr/local/lib -lgtest -lgtest_main -pthread
    endif
endif

# Add GTest include to CXXFLAGS
CXXFLAGS += $(GTEST_INCLUDE)

# Build settings
BUILD_DIR = build
TARGET = chunker

# Source files
SRCS = main.cpp
HEADERS = chunk.hpp data_structures.hpp utils.hpp config.hpp

# Object files
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

# Test settings
TEST_DIR = tests
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_TARGET = $(BUILD_DIR)/run_tests

# Default target
all: $(BUILD_DIR)/$(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile the executable
$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

# Compile object files
$(BUILD_DIR)/%.o: %.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Run the program
run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

# Test target
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) | $(BUILD_DIR)/$(TEST_DIR)
	$(CXX) $(TEST_OBJS) $(GTEST_LIBS) -o $@

$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)/$(TEST_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TEST_DIR):
	mkdir -p $(BUILD_DIR)/$(TEST_DIR)

# Format settings
CLANG_FORMAT ?= clang-format
CLANG_FORMAT_STYLE = file  # Uses .clang-format if present, otherwise LLVM style

# Format source files
format:
	@which $(CLANG_FORMAT) > /dev/null || (echo "$(CLANG_FORMAT) not found. Please install it."; exit 1)
	$(CLANG_FORMAT) -i -style=$(CLANG_FORMAT_STYLE) *.cpp *.hpp $(TEST_DIR)/*.cpp

# Check format (useful for CI)
format-check:
	@which $(CLANG_FORMAT) > /dev/null || (echo "$(CLANG_FORMAT) not found. Please install it."; exit 1)
	$(CLANG_FORMAT) -style=$(CLANG_FORMAT_STYLE) -n -Werror *.cpp *.hpp $(TEST_DIR)/*.cpp

# Documentation settings
DOXYGEN = doxygen
DOXYFILE = Doxyfile
PYTHON ?= python3
DOC_PORT ?= 8080

# Generate documentation
docs:
	@which $(DOXYGEN) > /dev/null || (echo "$(DOXYGEN) not found. Please install it."; exit 1)
	mkdir -p build/docs
	$(DOXYGEN) $(DOXYFILE)

# Serve documentation locally
serve-docs: docs
	@which $(PYTHON) > /dev/null || (echo "$(PYTHON) not found. Please install it."; exit 1)
	@echo "Serving documentation at http://localhost:$(DOC_PORT)"
	@cd build/docs/html && $(PYTHON) -m http.server $(DOC_PORT)

# Phony targets
.PHONY: all clean run test format format-check docs serve-docs

VERSION = 1.0
PACKAGE = myproject

# Create distribution package
dist:
	mkdir -p $(PACKAGE)-$(VERSION)
	cp -r src include Makefile LICENSE README.md $(PACKAGE)-$(VERSION)/
	tar -czf $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION)
	rm -rf $(PACKAGE)-$(VERSION)

# Run distribution tests
distcheck: dist
	@echo "Testing distribution package..."
	@tmp_dir=`mktemp -d` && \
	tar -C $$tmp_dir -xzf $(PACKAGE)-$(VERSION).tar.gz && \
	cd $$tmp_dir/$(PACKAGE)-$(VERSION) && \
	$(MAKE) && \
	$(MAKE) check && \
	$(MAKE) install DESTDIR=$$tmp_dir/_inst && \
	$(MAKE) uninstall DESTDIR=$$tmp_dir/_inst && \
	cd - > /dev/null && \
	rm -rf $$tmp_dir && \
	echo "Distribution package test passed!"

.PHONY: dist distcheck