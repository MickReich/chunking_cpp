#include "chunk_visualization.hpp"
#include <string>
#include <vector>

namespace chunk_viz {
    // Only instantiate the entire class for each type
    template class ChunkVisualizer<std::vector<double>>;
    template class ChunkVisualizer<std::vector<float>>;
    template class ChunkVisualizer<std::vector<int>>;
    template class ChunkVisualizer<double>;
    template class ChunkVisualizer<float>;
    template class ChunkVisualizer<int>;
} // namespace chunk_viz 