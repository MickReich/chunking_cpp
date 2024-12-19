# Building Instructions

## Prerequisites

- C++17 compiler (g++ or clang++)
- Make
- Google Test (for testing)
- Doxygen (for documentation)
- Graphviz (for documentation graphs)

## Installing Dependencies

### Ubuntu/Debian
```bash
# Install build tools
sudo apt-get update
sudo apt-get install g++ make

# Install Google Test
sudo apt-get install libgtest-dev cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib

# Install documentation tools
sudo apt-get install doxygen graphviz
```

### macOS
```bash
# Install build tools
xcode-select --install

# Install Google Test
brew install googletest

# Install documentation tools
brew install doxygen graphviz
```

### Windows
1. Install MinGW or Visual Studio
2. Install vcpkg
3. Install Google Test through vcpkg:
```bash
vcpkg install gtest:x64-windows
```

## Building the Project

1. Clone the repository:
```bash
git clone <repository-url>
cd <repository-name>
```

2. Build the project:
```bash
make
```

3. Run the tests:
```bash
make test
```

4. Generate documentation:
```bash
make docs
```

5. Serve documentation locally:
```bash
make docs-serve
```

## Project Structure
```
.
├── include/          # Header files
│   ├── chunk.hpp
│   ├── config.hpp
│   ├── data_structures.hpp
│   └── utils.hpp
├── src/             # Source files
│   └── main.cpp
├── tests/           # Test files
│   ├── test_main.cpp
│   ├── chunk_test.cpp
│   ├── data_structures_test.cpp
│   └── utils_test.cpp
├── build/           # Build artifacts
├── docs/           # Generated documentation
├── Makefile
├── README.md
└── BUILDING.md
```

## Make Targets

- `make`: Build the project
- `make test`: Run all tests
- `make test-<name>`: Run specific test suite
- `make docs`: Generate documentation
- `make docs-serve`: Serve documentation locally
- `make clean`: Clean build artifacts
- `make format`: Format source code
- `make format-check`: Check source code formatting 