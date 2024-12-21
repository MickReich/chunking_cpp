# Advanced Chunk Processing Library

[![codecov](https://codecov.io/gh/JohnnyTeutonic/chunking_cpp/branch/main/graph/badge.svg)](https://codecov.io/gh/JohnnyTeutonic/chunking_cpp/)
[![Tests](https://github.com/JohnnyTeutonic/chunking_cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/JohnnyTeutonic/chunking_cpp/actions/workflows/ci.yml)

A modern C++17 library providing advanced data chunking and processing capabilities with a focus on performance and flexibility.

## Overview

This library offers a comprehensive suite of tools for handling data in chunks, with features ranging from basic chunking to advanced parallel processing and compression techniques. It's designed to be efficient, type-safe, and easy to integrate into existing C++ projects.

## Features

### Core Functionality
- Parallel chunk processing
- Chunk compression (RLE, Delta)
- Basic chunking operations

### Advanced Chunking Strategies
- Overlapping chunks
- Predicate-based chunking
- Sum-based chunking
- Equal division chunking
- Sliding window chunking
- Statistical threshold-based chunking
- Similarity-based chunking
- Monotonicity-based chunking
- Padded fixed-size chunking

### Data Structures
- Circular buffer
- Sliding window
- Priority queue
- Chunk list

### Analysis & Utilities
- Statistical operations
- Random chunk generation
- Chunk manipulation utilities

## Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10 or higher
- Google Test (for testing)
- Doxygen (for documentation)
- Graphviz (for documentation graphs)

## Quick Start

```bash
# Clone the repository
git clone git@github.com:JohnnyTeutonic/chunking_cpp.git
cd chunking_cpp

# Configure the project (optionally with tests, documentation, sanitizers, and coverage enabled)
chmod +x configure
./configure  --enable-tests --enable-docs --enable-sanitizers --enable-coverage

# Build the project
make

# To execute the example program
make run
# Run tests
make test
# Uninstall if needed
make uninstall
```

For detailed build instructions, dependencies, and configuration options, see [BUILDING.md](BUILDING.md).

## Usage Examples

### Basic Usage
```cpp
#include "chunk.hpp"

// Create a chunk processor with size 2
Chunk<int> chunker(2);

// Add data
std::vector<int> data = {1, 2, 3, 4, 5};
chunker.add(data);

// Get fixed-size chunks
auto chunks = chunker.get_chunks(); // Returns: {{1,2}, {3,4}, {5}}
```

### Advanced Features
```cpp
// Predicate-based chunking
auto even_chunks = chunker.chunk_by_predicate([](int x) { return x % 2 == 0; });

// Similarity-based chunking
auto similar_chunks = chunker.chunk_by_similarity(3);

// Parallel processing
ParallelChunkProcessor<int> parallel_chunker(4); // 4 threads
parallel_chunker.process_chunks(chunks, [](int x) { return x * 2; });
```

See `src/main.cpp` for more comprehensive examples.

## Documentation

The library is extensively documented using Doxygen. You can:

1. View the online documentation at [GitHub Pages](https://johnnyteutonic.github.io/chunking_cpp/)
2. Generate documentation locally:

```bash
make docs
make docs-serve
```

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Write tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

## Citation

If you use this library in your research, please cite:

```bibtex
@software{chunking_cpp,
  author = {Reich, Jonathan},
  title = {Advanced Chunk Processing Library},
  year = {2024},
  url = {https://github.com/JohnnyTeutonic/chunking_cpp}
}
```

## Project Structure

```markdown
.
├── include/
│   ├── core/
│   │   ├── chunk.hpp
│   │   └── config.hpp
│   ├── strategies/
│   │   ├── chunk_strategies.hpp
│   │   └── chunk_compression.hpp
│   ├── parallel/
│   │   └── parallel_chunk.hpp
│   ├── structures/
│   │   ├── advanced_structures.hpp
│   │   └── data_structures.hpp
│   └── utils/
│       └── utils.hpp
├── src/
│   └── main.cpp
├── tests/
│   ├── test_main.cpp
│   ├── chunk_test.cpp
│   ├── chunk_strategies_test.cpp
│   ├── chunk_compression_test.cpp
│   ├── parallel_chunk_test.cpp
│   ├── advanced_structures_test.cpp
│   ├── data_structures_test.cpp
│   └── utils_test.cpp
├── scripts/
│   └── run_tests.sh
├── .github/
│   └── workflows/
│       └── ci.yml
├── Makefile
├── CMakeLists.txt
├── Doxyfile
├── README.md
├── BUILDING.md
└── LICENSE
```
