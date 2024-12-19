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
VERSION = 1.0
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

# Phony targets
.PHONY: all clean run dist distcheck test test-%