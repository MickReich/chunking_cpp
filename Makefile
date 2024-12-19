# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I.

# Build settings
BUILD_DIR = build
TARGET = chunker

# Source files
SRCS = main.cpp
HEADERS = chunk.hpp data_structures.hpp utils.hpp config.hpp

# Object files
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

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

# Format source files
format:
	clang-format -i *.cpp *.hpp

# Generate documentation
docs:
	doxygen Doxyfile

# Phony targets
.PHONY: all clean run format docs