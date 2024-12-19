# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I./include

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

# Directory settings
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin

# Target executable
TARGET = $(BIN_DIR)/chunker

# Source and object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Header files
HEADERS = $(wildcard $(INC_DIR)/*.hpp)

# Test settings
TEST_DIR = tests
TEST_BIN_DIR = $(BUILD_DIR)/tests
TEST_SRCS = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS = $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(TEST_BIN_DIR)/%.o)
TEST_BINS = $(TEST_BIN_DIR)/test_runner

# GTest settings
GTEST_LIBS = -lgtest -lgtest_main -pthread
TEST_CXXFLAGS = $(CXXFLAGS)

# Documentation settings
DOCS_DIR = $(BUILD_DIR)/docs
HTML_DIR = $(DOCS_DIR)/html
DOXYGEN = doxygen
DOXYFILE = Doxyfile
PYTHON ?= python3
DOC_PORT ?= 8080

# Default target
all: $(TARGET)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile the executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test targets
$(TEST_BIN_DIR):
	mkdir -p $(TEST_BIN_DIR)

$(TEST_BIN_DIR)/%.o: $(TEST_DIR)/%.cpp $(HEADERS) | $(TEST_BIN_DIR)
	$(CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(TEST_BIN_DIR)/test_runner: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) $(GTEST_LIBS) -o $@

.PRECIOUS: $(TEST_BIN_DIR)/%.o

test: $(TEST_BINS)
	@echo "Running all tests..."
	./$(TEST_BINS)

# Run specific test suite (e.g., make test-chunk)
test-%: $(TEST_BINS)
	./$(TEST_BINS) --gtest_filter=$**

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(PACKAGE)-$(VERSION).tar.gz

# Run the program
run: $(TARGET)
	$(TARGET)

# Distribution package
VERSION = 0.1
PACKAGE = vector-chunker

dist:
	mkdir -p $(PACKAGE)-$(VERSION)
	cp -r $(SRC_DIR) $(INC_DIR) Makefile BUILDING.md LICENSE README.md $(PACKAGE)-$(VERSION)/
	tar -czf $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION)
	rm -rf $(PACKAGE)-$(VERSION)

# Distribution check
distcheck: dist
	@echo "Testing distribution package..."
	@tmp_dir=`mktemp -d` && \
	tar -C $$tmp_dir -xzf $(PACKAGE)-$(VERSION).tar.gz && \
	cd $$tmp_dir/$(PACKAGE)-$(VERSION) && \
	$(MAKE) && \
	$(MAKE) run && \
	cd - > /dev/null && \
	rm -rf $$tmp_dir && \
	echo "Distribution package test passed!"


CLANG_FORMAT ?= clang-format
CLANG_FORMAT_STYLE = file  # Uses .clang-format if present, otherwise LLVM style

# Format source files
format:
	@which $(CLANG_FORMAT) > /dev/null || (echo "$(CLANG_FORMAT) not found. Please install it."; exit 1)
	$(CLANG_FORMAT) -i -style=$(CLANG_FORMAT_STYLE) $(SRC_DIR)/*.cpp $(INC_DIR)/*.hpp $(TEST_DIR)/*.cpp

# Check format (useful for CI)
format-check:
	@which $(CLANG_FORMAT) > /dev/null || (echo "$(CLANG_FORMAT) not found. Please install it."; exit 1)
	$(CLANG_FORMAT) -style=$(CLANG_FORMAT_STYLE) -n -Werror $(SRC_DIR)/*.cpp $(INC_DIR)/*.hpp $(TEST_DIR)/*.cpp

# Documentation targets
docs: | $(DOCS_DIR)
	@which $(DOXYGEN) > /dev/null || (echo "$(DOXYGEN) not found. Please install it."; exit 1)
	mkdir -p build/docs
	$(DOXYGEN) $(DOXYFILE)

# Clean documentation
docs-clean:
	rm -rf $(DOCS_DIR)

# Serve documentation locally
docs-serve: docs
	@which $(PYTHON) > /dev/null || (echo "$(PYTHON) not found. Please install it."; exit 1)
	@echo "Serving documentation at http://localhost:$(DOC_PORT)"
	@cd $(DOCS_DIR)/html && $(PYTHON) -m http.server $(DOC_PORT)

$(DOCS_DIR):
	mkdir -p $(DOCS_DIR)

# Phony targets
.PHONY: all clean run dist distcheck test test-% format format-check docs docs-clean docs-serve
