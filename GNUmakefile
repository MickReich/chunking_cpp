# Project configuration
PROJECT_NAME := ChunkProcessor
BUILD_DIR := build
DOC_DIR := $(BUILD_DIR)/docs
TEST_DIR := $(BUILD_DIR)/tests

# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra

# Ensure build directory exists and is configured
.PHONY: setup-build test docs docs-clean docs-serve local-help run uninstall

setup-build:
	@mkdir -p $(BUILD_DIR)
	@echo "Configuring CMake..."
	@cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
	@echo "Building project..."
	@cmake --build $(BUILD_DIR) --config Debug

# Run the executable
run: setup-build
	@echo "Running program..."
	@$(BUILD_DIR)/chunk_processor_exe

# Override CMake's test target
test: setup-build
	@echo "Running tests..."
	@if [ -f "$(BUILD_DIR)/run_tests" ]; then \
		$(BUILD_DIR)/run_tests; \
	elif [ -f "$(BUILD_DIR)/tests/run_tests" ]; then \
		$(BUILD_DIR)/tests/run_tests; \
	else \
		echo "Error: run_tests executable not found"; \
		echo "Checking in:"; \
		echo "  - $(BUILD_DIR)/run_tests"; \
		echo "  - $(BUILD_DIR)/tests/run_tests"; \
		echo "Build failed. Try running 'make clean' followed by 'make'"; \
		exit 1; \
	fi

# Clean build artifacts
clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

# Alias for clean
uninstall: clean

# Documentation targets
docs: setup-build
	@if ! command -v doxygen > /dev/null; then \
		echo "Error: doxygen not found. Please install doxygen first."; \
		exit 1; \
	fi
	@echo "Creating documentation directories..."
	@mkdir -p $(DOC_DIR)
	@chmod 755 $(DOC_DIR)
	@doxygen Doxyfile
	@echo "Documentation generated in $(DOC_DIR)/html"

# Clean documentation
docs-clean:
	@rm -rf $(DOC_DIR)
	@echo "Cleaned documentation directory"

# Serve documentation locally
docs-serve: docs
	@if ! command -v python3 > /dev/null; then \
		echo "Error: python3 not found. Please install python3 first."; \
		exit 1; \
	fi
	@echo "Serving documentation at http://localhost:8000"
	@cd $(DOC_DIR)/html && python3 -m http.server 8000

# Help target
local-help:
	@echo "Custom targets available:"
	@echo "  setup-build  - Configure and build the project"
	@echo "  test         - Run tests"
	@echo "  docs         - Generate documentation"
	@echo "  docs-clean   - Remove generated documentation"
	@echo "  docs-serve   - Serve documentation locally at http://localhost:8000"
	@echo "  uninstall    - Remove all build artifacts (alias for clean)"
	@echo "  local-help   - Show this help message"
	@echo
	@echo "For CMake targets, run 'make help'"

# Default target
.DEFAULT_GOAL := setup-build