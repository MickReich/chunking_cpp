# Advanced Chunk Processing Library

A C++ library for flexible data chunking and processing operations.

## Features

- Parallel chunk processing
- Chunk compression (RLE, Delta)
- Basic chunking operations
- Overlapping chunks
- Predicate-based chunking
- Sum-based chunking
- Equal division chunking
- Sliding window chunking
- Statistical threshold-based chunking
- Similarity-based chunking
- Monotonicity-based chunking
- Padded fixed-size chunking
- Circular buffer
- Sliding window
- Priority queue
- Chunk list
- Statistical operations
- Random chunk generation
- Chunk manipulation utilities

## Quick Start

```bash
# Clone the repository
git clone git@github.com:JohnnyTeutonic/chunking_cpp.git
cd chunking_cpp

# Configure the project (with tests and documentation enabled)
chmod +x configure
./configure  --enable-tests --enable-docs

# Build the project
make

# To execute the main program
./chunk_processor_exe

# Uninstall if needed
make uninstall

# Run tests
make test
```

For detailed build instructions, dependencies, and configuration options, see [BUILDING.md](BUILDING.md).

## Usage Examples

See `main.cpp` for comprehensive examples of all features.

## Documentation

Generate and view the documentation locally:

```bash
make docs
make docs-serve
```

## License

This project is licensed under the GNU General Public License v2.0 - see the [LICENSE](LICENSE) file for details.

## Project Structure

```markdown
.
├── include/
│   ├── chunk.hpp
│   ├── config.hpp
│   ├── advanced_structures.hpp
│   ├── chunk_compression.hpp
│   ├── chunk_strategies.hpp
│   ├── parallel_chunk.hpp
│   └── utils.hpp
├── src/
│   └── main.cpp
├── tests/
│   ├── test_main.cpp
│   ├── chunk_test.cpp
│   ├── chunk_strategies_test.cpp
│   ├── chunk_compression_test.cpp
│   ├── parallel_chunk_test.cpp
│   ├── advanced_structures_test.cpp
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
