#include "gpu_chunking.hpp"

int main() {
    // Check GPU availability
    if (!gpu_chunking::GPUChunking<int>::is_gpu_available()) {
        std::cerr << "No GPU available\n";
        return 1;
    }

    // Print GPU info
    std::cout << gpu_chunking::GPUChunking<int>::get_gpu_info() << std::endl;

    // Create chunker
    gpu_chunking::GPUChunking<int> chunker(32, 0.1f);

    std::vector<int> data = {1, 2, 3, 10, 11, 12, 4, 5, 6};

    // Perform chunking
    auto chunks = chunker.chunk(data);
}