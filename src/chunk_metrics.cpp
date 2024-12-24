#include "chunk_metrics.hpp"
#include <string>
#include <vector>

namespace chunk_metrics {
    // Only instantiate the entire class for each type
    template class ChunkQualityAnalyzer<double>;
    template class ChunkQualityAnalyzer<float>;
    template class ChunkQualityAnalyzer<int>;
} // namespace chunk_metrics 