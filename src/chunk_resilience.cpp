#include "chunk_resilience.hpp"
#include "chunk_errors.hpp"

namespace chunk_resilience {

// Template instantiations
template class Checkpoint<double>;
template class Checkpoint<float>;
template class Checkpoint<int>;

template class ResilientChunker<double>;
template class ResilientChunker<float>;
template class ResilientChunker<int>;

} // namespace chunk_resilience