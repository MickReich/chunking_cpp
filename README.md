# Project Requirements

## Build Tools

- CMake (version 3.15 or higher)
- g++ or compatible C++ compiler with C++11 support
- Make

## Package Manager

- vcpkg (for managing C++ libraries)

## Dependencies

Currently, this project has no external library dependencies.

## Setup Instructions

1. Install Build Tools:

   ```bash
   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get install build-essential cmake

   # macOS (using Homebrew)
   brew install cmake

   # Windows
   # Download and install CMake from https://cmake.org/download/
   # Install Visual Studio with C++ development tools
   ```

2. Install vcpkg (optional - for future dependencies):

   ```bash
   # Clone vcpkg
   git clone https://github.com/Microsoft/vcpkg.git
   
   # Run the bootstrap script
   # Linux/macOS
   ./vcpkg/bootstrap-vcpkg.sh
   
   # Windows
   .\vcpkg\bootstrap-vcpkg.bat
   ```

3. Build the Project:

   ```bash
   # Create and enter build directory
   mkdir build && cd build
   
   # Build using make
   make
   ```
