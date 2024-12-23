/**
 * @file chunk_visualization_demo.cpp
 * @brief Demonstration of chunk visualization capabilities
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#include "chunk_visualization.hpp"
#include <iostream>
#include <random>
#include <vector>

/**
 * @brief Generate sample data with natural breakpoints for visualization
 * @return Vector of vectors containing sample chunk data
 */
std::vector<std::vector<double>> generate_sample_chunks() {
    std::vector<std::vector<double>> chunks;
    std::random_device rd;
    std::mt19937 gen(rd());

    // Create normal distributions with different means
    std::normal_distribution<double> dist1(0.0, 1.0);
    std::normal_distribution<double> dist2(5.0, 1.0);
    std::normal_distribution<double> dist3(-3.0, 1.0);

    // Generate chunks with different characteristics
    std::vector<double> chunk1, chunk2, chunk3;

    // First chunk: 10 values around mean 0
    for (int i = 0; i < 10; ++i) {
        chunk1.push_back(dist1(gen));
    }
    chunks.push_back(chunk1);

    // Second chunk: 15 values around mean 5
    for (int i = 0; i < 15; ++i) {
        chunk2.push_back(dist2(gen));
    }
    chunks.push_back(chunk2);

    // Third chunk: 8 values around mean -3
    for (int i = 0; i < 8; ++i) {
        chunk3.push_back(dist3(gen));
    }
    chunks.push_back(chunk3);

    return chunks;
}

/**
 * @brief Main function demonstrating chunk visualization features
 * @return 0 on success, 1 on error
 */
int main() {
    try {
        // Create output directory
        const std::string viz_dir = "./viz";
        std::filesystem::create_directories(viz_dir);

        // Generate sample data
        auto chunks = generate_sample_chunks();
        std::vector<double> flattened_data;
        for (const auto& chunk : chunks) {
            flattened_data.insert(flattened_data.end(), chunk.begin(), chunk.end());
        }

        // Create visualizer
        chunk_viz::ChunkVisualizer<std::vector<double>> visualizer(chunks, viz_dir);

        // 1. Plot chunk sizes
        std::cout << "Generating chunk size plot..." << std::endl;
        visualizer.plot_chunk_sizes();
        std::cout << "Chunk size plot saved to " << viz_dir << "/chunk_sizes.png" << std::endl;

        // 2. Visualize chunk boundaries
        std::cout << "\nGenerating boundary visualization..." << std::endl;
        visualizer.visualize_boundaries();
        std::cout << "Boundary visualization saved to " << viz_dir << "/boundaries.txt"
                  << std::endl;

        // 3. Export to GraphViz
        std::cout << "\nGenerating GraphViz visualization..." << std::endl;
        visualizer.export_to_graphviz();
        std::cout << "GraphViz file saved to " << viz_dir << "/chunks.dot" << std::endl;
        std::cout << "To generate PNG, run: dot -Tpng " << viz_dir << "/chunks.dot -o " << viz_dir
                  << "/chunks.png" << std::endl;

        // Create a second visualization with single values
        std::cout << "\nCreating alternative visualization with individual values..." << std::endl;
        chunk_viz::ChunkVisualizer<double> single_visualizer(flattened_data, viz_dir + "/single");
        single_visualizer.plot_chunk_sizes();
        single_visualizer.visualize_boundaries();
        single_visualizer.export_to_graphviz(viz_dir + "/single/individual_chunks.dot");

        std::cout << "\nAll visualizations have been generated successfully!" << std::endl;
        std::cout << "Check the '" << viz_dir << "' directory for the output files." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}