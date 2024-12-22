/*Copyright (C) 2024  Jonathan Reich
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License along
with this program; if not, see <https://www.gnu.org/licenses/>.
*/

/**
 * @file main.cpp
 * @brief Demonstrates various chunking strategies and operations.
 *
 * This file contains examples of how to use the chunking library to process
 * data in different ways, including integer, float, and string chunking.
 */

#include "advanced_structures.hpp"
#include "chunk.hpp"
#include "chunk_compression.hpp"
#include "chunk_strategies.hpp"
#include "chunk_windows.hpp"
#include "config.hpp"
#include "data_structures.hpp"
#include "parallel_chunk.hpp"
#include "sub_chunk_strategies.hpp"
#include "utils.hpp"
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

using namespace advanced_structures; // For ChunkSkipList and ChunkBPlusTree
using namespace parallel_chunk;      // For ParallelChunkProcessor
using namespace chunk_compression;   // For ChunkCompressor
using namespace chunk_strategies;    // For QuantileStrategy, VarianceStrategy, etc.
using namespace chunk_windows;

// Helper function to print chunks
template <typename T>
void print_chunks(const std::vector<std::vector<T>>& chunks) {
    std::cout << "Chunks: [" << std::endl;
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << "  " << i << ": [";
        for (const auto& value : chunks[i]) {
            std::cout << std::fixed << std::setprecision(2) << value << " ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << "]" << std::endl;
}

/**
 * @brief Helper function to print sub-chunks with detailed formatting
 * @tparam T The type of elements in the chunks
 * @param sub_chunks The 3D vector containing the sub-chunks to print
 * @param label The label to display for this set of sub-chunks
 * @param precision The number of decimal places to show for floating-point numbers
 */
template <typename T>
void print_sub_chunks(const std::vector<std::vector<std::vector<T>>>& sub_chunks,
                      const std::string& label, int precision = 2) {
    std::cout << "\n" << label << ":\n";
    for (size_t i = 0; i < sub_chunks.size(); ++i) {
        std::cout << "Level " << i + 1 << ":\n";
        for (size_t j = 0; j < sub_chunks[i].size(); ++j) {
            std::cout << "  Sub-chunk " << j + 1 << ": ";
            for (const auto& val : sub_chunks[i][j]) {
                std::cout << std::fixed << std::setprecision(precision) << val << " ";
            }
            std::cout << "\n";
        }
    }
}

/**
 * @brief Demonstrates complex recursive sub-chunking with multiple levels
 *
 * This function shows how to apply recursive sub-chunking strategies
 * to data with clear patterns, using variance-based chunking at multiple levels.
 */
void demonstrate_complex_recursive_subchunking() {
    std::cout << "\n=== Complex Recursive Sub-chunking ===" << std::endl;

    // Example data
    std::vector<std::vector<double>> data = {{1.0, 1.1, 1.2, 5.0, 5.1, 5.2},
                                             {2.0, 2.1, 2.2, 10.0, 10.1, 10.2},
                                             {3.0, 3.1, 3.2, 15.0, 15.1, 15.2}};

    // Recursive sub-chunking
    auto variance_strategy = std::make_shared<VarianceStrategy<double>>(3.0);
    RecursiveSubChunkStrategy<double> recursive_strategy(variance_strategy, 3, 2);
    auto recursive_result = recursive_strategy.apply(data);

    print_sub_chunks(recursive_result, "Recursive Sub-chunking");
}

/**
 * @brief Demonstrates hierarchical sub-chunking using multiple strategies
 *
 * This function shows how to apply different chunking strategies
 * in a hierarchical manner, combining variance, similarity, and entropy-based approaches.
 */
void demonstrate_multi_strategy_subchunking() {
    std::cout << "\n=== Multi-Strategy Sub-chunking ===" << std::endl;

    // Example data
    std::vector<std::vector<double>> data = {{1.0, 1.1, 1.2, 5.0, 5.1, 5.2},
                                             {2.0, 2.1, 2.2, 10.0, 10.1, 10.2},
                                             {3.0, 3.1, 3.2, 15.0, 15.1, 15.2}};

    // Hierarchical sub-chunking
    std::vector<std::shared_ptr<ChunkStrategy<double>>> strategies = {
        std::make_shared<VarianceStrategy<double>>(5.0),
        std::make_shared<EntropyStrategy<double>>(1.0)};
    HierarchicalSubChunkStrategy<double> hierarchical_strategy(strategies, 2);
    auto hierarchical_result = hierarchical_strategy.apply(data);

    print_sub_chunks(hierarchical_result, "Hierarchical Sub-chunking");
}

/**
 * @brief Demonstrates adaptive conditional sub-chunking
 *
 * This function shows how to use conditional sub-chunking with
 * adaptive thresholds based on chunk properties.
 */
void demonstrate_adaptive_conditional_subchunking() {
    std::cout << "\n=== Adaptive Conditional Sub-chunking ===" << std::endl;

    // Example data
    std::vector<std::vector<double>> data = {{1.0, 1.1, 1.2, 5.0, 5.1, 5.2},
                                             {2.0, 2.1, 2.2, 10.0, 10.1, 10.2},
                                             {3.0, 3.1, 3.2, 15.0, 15.1, 15.2}};

    // Conditional sub-chunking
    auto condition = [](const std::vector<double>& chunk) {
        double mean = std::accumulate(chunk.begin(), chunk.end(), 0.0) / chunk.size();
        double variance = 0.0;
        for (const auto& val : chunk) {
            variance += (val - mean) * (val - mean);
        }
        variance /= chunk.size();
        return variance > 50.0;
    };

    auto variance_strategy = std::make_shared<VarianceStrategy<double>>(5.0);
    ConditionalSubChunkStrategy<double> conditional_strategy(variance_strategy, condition, 2);
    auto conditional_result = conditional_strategy.apply(data);

    print_sub_chunks(conditional_result, "Conditional Sub-chunking");
}

/**
 * @brief Main function demonstrating various chunking strategies
 * @return 0 on successful execution
 */
int main(int argc, char* argv[]) {
    demonstrate_complex_recursive_subchunking();
    demonstrate_multi_strategy_subchunking();
    demonstrate_adaptive_conditional_subchunking();

    std::cout << "\n=== Demonstrating Advanced Chunking Structures ===\n";

    // Binary data example
    std::vector<uint8_t> binary_data = {0xFF, 0x00, 0xAA, 0x55, 0xCC};
    AdaptiveChunkTree<uint8_t> binary_chunker;
    auto binary_chunks = binary_chunker.chunk(binary_data);
    std::cout << "Binary chunks created: " << binary_chunks.size() << "\n";

    // Character data example
    std::string char_data = "Hello, World!";
    AdaptiveChunkTree<char> char_chunker;
    auto char_chunks = char_chunker.chunk(std::vector<char>(char_data.begin(), char_data.end()));
    std::cout << "Character chunks created: " << char_chunks.size() << "\n";

    // Float data example
    std::vector<float> float_data = {1.0f, 2.5f, 3.7f, 4.2f, 5.9f};
    AdaptiveChunkTree<float> float_chunker;
    auto float_chunks = float_chunker.chunk(float_data);
    std::cout << "Float chunks created: " << float_chunks.size() << "\n";

    // Example: SemanticChunker usage
    std::cout << "\n=== SemanticChunker Example ===" << std::endl;
    SemanticChunker<std::string> text_chunker;
    std::string text = "This is the first sentence. This is the second one. And here's a third!";
    auto text_chunks = text_chunker.chunk(text);
    std::cout << "Text chunks created: " << text_chunks.size() << "\n";

    // Custom NLP model example
    class CustomNLPModel {
    public:
        double calculateSimilarity(const std::string& s1, const std::string& s2) {
            // Simple example: compare lengths as a similarity metric
            return std::abs(1.0 - static_cast<double>(std::abs(static_cast<int>(s1.length()) - static_cast<int>(s2.length()))) / std::max(s1.length(), s2.length()));
        }
    };

    SemanticChunker<std::string, CustomNLPModel> custom_chunker;
    auto custom_chunks = custom_chunker.chunk(text);
    std::cout << "Custom model chunks created: " << custom_chunks.size() << "\n\n";

    return 0;
}
