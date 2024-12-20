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
sudo ln -s /usr/src/gtest/include/gtest /usr/include/gtest

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

1.Clone the repository:

```bash
git clone git@github.com:JohnnyTeutonic/chunking_cpp.git
cd chunking_cpp
```

2.Conversion of the configure script to unix line endings (Windows only):

```bash

unix2dos configure
```

Make configure script executable

```bash
chmod +x configure
```

3.Configure the build:

```bash
./configure
```

Configuration options:

- `--prefix=PATH`: Set installation prefix (default: /usr/local)
- `--build-type=TYPE`: Set build type (Debug|Release)
- `--disable-tests`: Disable building tests
- `--disable-docs`: Disable building documentation

example:

```bash
./configure --prefix=/usr/local --build-type=Debug --disable-tests --disable-docs
```

3.Build and install the project:

```bash
make install
```

4.Run the tests:

```bash
make test
```

5.Generate documentation:

```bash
make docs
```

6.Serve documentation locally:

```bash
make docs-serve
```

## Project Structure

```markdown
.
├── include/          # Header files
│   ├── chunk.hpp
│   ├── config.hpp
│   ├── advanced_structures.hpp
│   ├── chunk_compression.hpp
│   ├── chunk_strategies.hpp
│   ├── parallel_chunk.hpp
│   └── utils.hpp
├── src/             # Source files
│   └── main.cpp
├── tests/           # Test files
│   ├── test_main.cpp
│   ├── chunk_test.cpp
│   ├── chunk_strategies_test.cpp
│   ├── chunk_compression_test.cpp
│   ├── parallel_chunk_test.cpp
│   ├── advanced_structures_test.cpp
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
- `make install`: Install the project
- `make uninstall`: Uninstall the project
