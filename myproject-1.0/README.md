# Advanced Chunk Processing Library

A C++ library for flexible data chunking and processing operations.

## Features

- Basic chunking operations
- Overlapping chunks
- Predicate-based chunking
- Sum-based chunking
- Equal division chunking
- Circular buffer
- Sliding window
- Priority queue
- Chunk list
- Statistical operations
- Random chunk generation
- Chunk manipulation utilities

## Building

```bash
make
```

## Running

```bash
make run
```

## Examples

See `main.cpp` for comprehensive examples of all features.

## Documentation

Generate documentation:

```bash
make docs
```

## Requirements

- C++11 or later
- Make
- CMake (optional)

## Testing

### Installing Google Test

Choose one of the following methods:

#### Method 1: System Package Manager (Linux/macOS)

On Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install cmake g++ libgtest-dev
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib
```

On macOS:

```bash
brew install googletest
```

#### Method 2: Using vcpkg (Cross-platform)

```bash
# Clone and bootstrap vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # or bootstrap-vcpkg.bat on Windows

# Install GTest
./vcpkg install gtest

# Set environment variable
export VCPKG_ROOT=/path/to/vcpkg  # Add to your .bashrc or .zshrc
```

### Running Tests

After installing Google Test:

```bash
make test
```
