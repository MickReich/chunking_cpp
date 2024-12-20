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

## Quick Start

```bash
# Clone the repository
git clone git@github.com:JohnnyTeutonic/chunking_cpp.git
cd chunking_cpp

# Configure and build
chmod +x configure
./configure
make

# Run tests
make test
```

For detailed build instructions, dependencies, and configuration options, see [BUILDING.md](BUILDING.md).

## Usage Examples

See `main.cpp` for comprehensive examples of all features.

## Documentation

Generate and view the documentation:
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
│   ├── data_structures.hpp
│   └── utils.hpp
├── src/
│   └── main.cpp
├── tests/
│   ├── test_main.cpp
│   ├── chunk_test.cpp
│   ├── data_structures_test.cpp
│   └── utils_test.cpp
├── Makefile
├── Doxyfile
└── README.md
```
