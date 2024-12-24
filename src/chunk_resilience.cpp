#include "chunk_resilience.hpp"
#include <string>
#include <vector>

namespace chunk_resilience {
    // Only instantiate the entire struct/class for each type
    template struct Checkpoint<double>;
    template struct Checkpoint<float>;
    template struct Checkpoint<int>;

    template class ResilientChunker<double>;
    template class ResilientChunker<float>;
    template class ResilientChunker<int>;
} // namespace chunk_resilience 