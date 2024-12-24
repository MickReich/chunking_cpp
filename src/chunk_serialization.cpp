#include "chunk_serialization.hpp"
#include <string>
#include <vector>

namespace chunk_serialization {
    // Only instantiate the entire class for each type
    template class ChunkSerializer<double>;
    template class ChunkSerializer<float>;
    template class ChunkSerializer<int>;
} // namespace chunk_serialization 