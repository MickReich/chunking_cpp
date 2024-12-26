/**
 * @file demo_neural_chunking.cpp
 * @brief Demonstration of neural network-based chunking capabilities
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#include "../include/chunk_visualization.hpp"
#include "../include/neural_chunking.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

void print_chunks(const std::vector<std::vector<int>>& chunks) {
    std::cout << "Chunks:" << std::endl;
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << "Chunk " << i << ": [";
        for (const auto& val : chunks[i]) {
            std::cout << std::setw(3) << val << " ";
        }
        std::cout << "]" << std::endl;
    }
}

int main() {
    // Create sample data with patterns
    std::vector<int> data;
    for (int i = 0; i < 100; ++i) {
        // Create patterns that should be recognized
        if (i % 10 < 5) {
            data.push_back(i % 5);
        } else {
            data.push_back(10 - (i % 5));
        }
    }

    // Create neural chunking instance
    neural_chunking::NeuralChunking<int> chunker(8, 0.5);

    std::cout << "Original data size: " << data.size() << std::endl;
    std::cout << "Window size: " << chunker.get_window_size() << std::endl;

    // Try different thresholds
    std::vector<double> thresholds = {0.3, 0.5, 0.7};

    for (double threshold : thresholds) {
        std::cout << "\nTesting with threshold: " << threshold << std::endl;
        chunker.set_threshold(threshold);

        auto chunks = chunker.chunk(data);
        print_chunks(chunks);

        // Use chunk visualizer
        chunk_viz::ChunkVisualizer<std::vector<int>> visualizer(chunks, "./viz");
        visualizer.plot_chunk_sizes();
        visualizer.visualize_boundaries();
        visualizer.export_to_graphviz("neural_chunks.dot");
    }

    return 0;
}