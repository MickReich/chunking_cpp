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

#include "advanced_structures.hpp"
#include "chunk.hpp"
#include "chunk_compression.hpp"
#include "chunk_strategies.hpp"
#include "config.hpp"
#include "data_structures.hpp"
#include "parallel_chunk.hpp"
#include "utils.hpp"
#include "chunk_windows.hpp"
#include <iomanip>
#include <iostream>
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

int main() {
    // Example 1: Integer chunking
    std::cout << "\n=== Integer Chunking Example ===" << std::endl;
    Chunk<int> int_chunker(4); // Chunks of size 4
    std::vector<int> int_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int_chunker.add(int_data);

    std::cout << "Total elements: " << int_chunker.size() << std::endl;
    std::cout << "Number of chunks: " << int_chunker.chunk_count() << std::endl;
    print_chunks(int_chunker.get_chunks());

    // Example 2: Float chunking with partial chunk
    std::cout << "\n=== Float Chunking Example ===" << std::endl;
    Chunk<float> float_chunker(3); // Chunks of size 3
    float_chunker.add({1.1f, 2.2f, 3.3f, 4.4f, 5.5f});
    print_chunks(float_chunker.get_chunks());

    // Example 3: Double chunking with individual additions
    std::cout << "\n=== Double Chunking Example ===" << std::endl;
    Chunk<double> double_chunker(2); // Chunks of size 2
    double_chunker.add(1.5);
    double_chunker.add(2.5);
    double_chunker.add(3.5);

    std::cout << "Getting specific chunks:" << std::endl;
    auto chunk0 = double_chunker.get_chunk(0);
    std::cout << "Chunk 0: [" << chunk0[0] << " " << chunk0[1] << "]" << std::endl;

    auto chunk1 = double_chunker.get_chunk(1);
    std::cout << "Chunk 1: [" << chunk1[0] << "]" << std::endl;

    // Example 4: Error handling
    std::cout << "\n=== Error Handling Example ===" << std::endl;
    try {
        Chunk<int> invalid_chunker(0); // Should throw exception
    } catch (const std::invalid_argument& e) {
        std::cout << "Expected error: " << e.what() << std::endl;
    }

    // Example 5: Overlapping chunks
    std::cout << "\n=== Overlapping Chunks Example ===" << std::endl;
    Chunk<int> overlap_chunker(4);
    std::vector<int> overlap_data = {1, 2, 3, 4, 5, 6, 7, 8};
    overlap_chunker.add(overlap_data);

    std::cout << "Chunks with 2-element overlap:" << std::endl;
    auto overlapping_chunks = overlap_chunker.get_overlapping_chunks(2);
    print_chunks(overlapping_chunks);

    // Example 6: Chunking by predicate (start new chunk on even numbers)
    std::cout << "\n=== Predicate-based Chunking Example ===" << std::endl;
    Chunk<int> pred_chunker(1); // Use minimum chunk size of 1 for predicate-based chunking
    std::vector<int> pred_data = {1, 2, 3, 4, 5, 6, 7, 8};
    pred_chunker.add(pred_data);

    auto predicate_chunks = pred_chunker.chunk_by_predicate([](int x) { return x % 2 == 0; });
    std::cout << "Chunks starting with even numbers:" << std::endl;
    print_chunks(predicate_chunks);

    // Example 7: Chunking by sum
    std::cout << "\n=== Sum-based Chunking Example ===" << std::endl;
    Chunk<int> sum_chunker(1); // Use minimum chunk size of 1 for sum-based chunking
    std::vector<int> sum_data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    sum_chunker.add(sum_data);

    auto sum_chunks = sum_chunker.chunk_by_sum(10);
    std::cout << "Chunks with sum <= 10:" << std::endl;
    print_chunks(sum_chunks);

    // Example 8: Equal division chunking
    std::cout << "\n=== Equal Division Chunking Example ===" << std::endl;
    Chunk<int> equal_chunker(1); // Use minimum chunk size of 1 for n-division chunking
    std::vector<int> equal_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    equal_chunker.add(equal_data);

    auto equal_chunks = equal_chunker.chunk_into_n(3);
    std::cout << "Data divided into 3 chunks:" << std::endl;
    print_chunks(equal_chunks);

    // Example 13: Monotonicity-based chunking
    std::cout << "\n=== Monotonicity-based Chunking Example ===" << std::endl;
    Chunk<int> mono_chunker(1);
    std::vector<int> mono_data = {1, 3, 5, 4, 2, 1, 7, 9, 8, 6};
    mono_chunker.add(mono_data);

    auto mono_chunks = mono_chunker.chunk_by_monotonicity();
    std::cout << "Chunks based on monotonicity changes:" << std::endl;
    print_chunks(mono_chunks);

    // Example 14: Statistical threshold chunking
    std::cout << "\n=== Statistical Threshold Chunking Example ===" << std::endl;
    Chunk<double> stat_chunker(1);
    std::vector<double> stat_data = {1.0, 1.5, 5.0, 5.5, 2.0, 2.5, 7.0, 7.5};
    stat_chunker.add(stat_data);

    auto mean = [](const std::vector<double>& v) {
        return std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    };

    auto stat_chunks = stat_chunker.chunk_by_statistic(3.0, mean);
    std::cout << "Chunks based on mean threshold (3.0):" << std::endl;
    print_chunks(stat_chunks);

    // Example 15: Similarity-based chunking
    std::cout << "\n=== Similarity-based Chunking Example ===" << std::endl;
    Chunk<int> sim_chunker(1);
    std::vector<int> sim_data = {1, 2, 10, 11, 2, 3, 20, 21};
    sim_chunker.add(sim_data);

    auto sim_chunks = sim_chunker.chunk_by_similarity(3);
    std::cout << "Chunks based on similarity (max diff = 3):" << std::endl;
    print_chunks(sim_chunks);

    // Example 16: Padded fixed-size chunking
    std::cout << "\n=== Padded Fixed-size Chunking Example ===" << std::endl;
    Chunk<int> pad_chunker(3); // Chunks of size 3
    std::vector<int> pad_data = {1, 2, 3, 4, 5, 6, 7, 8};
    pad_chunker.add(pad_data);

    auto padded_chunks = pad_chunker.get_padded_chunks(0); // Pad with zeros
    std::cout << "Fixed-size chunks with padding:" << std::endl;
    print_chunks(padded_chunks);

    // Example 17: Combined strategies
    std::cout << "\n=== Combined Chunking Strategies Example ===" << std::endl;
    Chunk<double> combined_chunker(4);
    std::vector<double> combined_data = {1.1, 1.2, 1.3, 5.1, 5.2, 5.3, 2.1, 2.2, 2.3};
    combined_chunker.add(combined_data);

    // First chunk by similarity
    auto step1_chunks = combined_chunker.chunk_by_similarity(1.0);
    std::cout << "Step 1 - Similarity-based chunks:" << std::endl;
    print_chunks(step1_chunks);

    // Then apply sliding window to each chunk
    std::cout << "Step 2 - Sliding windows within each chunk:" << std::endl;
    for (size_t i = 0; i < step1_chunks.size(); ++i) {
        Chunk<double> window_chunker(2);
        window_chunker.add(step1_chunks[i]);
        auto windows = window_chunker.sliding_window(2, 1);
        std::cout << "Windows for chunk " << i << ":" << std::endl;
        print_chunks(windows);
    }

    // Example 18: Advanced chunking strategies
    std::cout << "\n=== Advanced Chunking Strategies Example ===" << std::endl;

    // Quantile-based chunking
    std::vector<double> advanced_data = {1.0, 2.0, 5.0, 6.0, 3.0, 4.0, 8.0, 7.0};
    QuantileStrategy<double> quantile_strategy(0.5);
    auto quantile_chunks = quantile_strategy.apply(advanced_data);
    std::cout << "Quantile-based chunks (median):" << std::endl;
    print_chunks(quantile_chunks);

    // Variance-based chunking
    VarianceStrategy<double> variance_strategy(1.0);
    auto variance_chunks = variance_strategy.apply(advanced_data);
    std::cout << "Variance-based chunks (threshold = 1.0):" << std::endl;
    print_chunks(variance_chunks);

    // Entropy-based chunking
    std::vector<int> entropy_data = {1, 1, 1, 2, 2, 3, 4, 4, 4, 4};
    EntropyStrategy<int> entropy_strategy(1.5);
    auto entropy_chunks = entropy_strategy.apply(entropy_data);
    std::cout << "Entropy-based chunks (threshold = 1.5):" << std::endl;
    print_chunks(entropy_chunks);

    // Example 19: Advanced Data Structures
    std::cout << "\n=== Advanced Data Structures Example ===" << std::endl;

    // Skip List example
    std::cout << "Skip List Operations:" << std::endl;
    ChunkSkipList<int> skip_list;
    std::vector<int> skip_data = {3, 6, 9, 2, 5, 8};
    for (int val : skip_data) {
        skip_list.insert(val);
    }

    std::cout << "Searching for values..." << std::endl;
    std::cout << "Value 6: " << (skip_list.search(6) ? "Found" : "Not found") << std::endl;
    std::cout << "Value 7: " << (skip_list.search(7) ? "Found" : "Not found") << std::endl;

    // B+ Tree example
    std::cout << "\nB+ Tree Operations:" << std::endl;
    ChunkBPlusTree<int> bplus_tree;
    for (int val : {1, 4, 7, 10, 13, 16}) {
        bplus_tree.insert(val);
    }

    // Example 20: Parallel Processing
    std::cout << "\n=== Parallel Processing Example ===" << std::endl;
    std::vector<std::vector<int>> parallel_data = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    // Double all numbers in parallel
    auto double_op = [](std::vector<int>& chunk) {
        for (int& val : chunk) {
            val *= 2;
        }
    };

    ParallelChunkProcessor<int>::process_chunks(parallel_data, double_op);
    std::cout << "After parallel doubling:" << std::endl;
    print_chunks(parallel_data);

    // Example 21: Chunk Compression
    std::cout << "\n=== Chunk Compression Example ===" << std::endl;
    std::vector<int> compress_data = {1, 1, 1, 2, 2, 3, 4, 4, 4, 4};

    // Run-length encoding
    auto rle = ChunkCompressor<int>::run_length_encode(compress_data);
    std::cout << "Run-length encoding:" << std::endl;
    for (const auto& [value, count] : rle) {
        std::cout << value << " appears " << count << " times" << std::endl;
    }

    // Delta encoding
    std::vector<int> sequence = {10, 12, 15, 19, 24};
    auto delta_encoded = ChunkCompressor<int>::delta_encode(sequence);
    std::cout << "\nDelta encoding:" << std::endl;
    std::cout << "Original: ";
    for (int val : sequence)
        std::cout << val << " ";
    std::cout << "\nEncoded:  ";
    for (int val : delta_encoded)
        std::cout << val << " ";
    std::cout << std::endl;

    // Example 22: Advanced Chunking Strategy Combinations
    std::cout << "\n=== Advanced Chunking Strategy Combinations ===" << std::endl;
    
    // Pattern-based chunking
    std::vector<int> pattern_data = {1, 2, 1, 2, 3, 4, 3, 4, 5, 6, 5, 6};
    Chunk<int> pattern_chunker(2);
    pattern_chunker.add(pattern_data);
    auto pattern_chunks = pattern_chunker.chunk_by_predicate([](int x) {
        static int last = x;
        bool start_new = (x <= last);
        last = x;
        return start_new;
    });
    std::cout << "Pattern-based chunks (new chunk when sequence decreases):" << std::endl;
    print_chunks(pattern_chunks);

    // Adaptive size chunking
    std::vector<double> adaptive_data = {1.0, 1.1, 5.0, 5.1, 5.2, 2.0, 2.1, 7.0, 7.1, 7.2};
    Chunk<double> adaptive_chunker(1);
    adaptive_chunker.add(adaptive_data);
    
    auto adaptive_chunks = adaptive_chunker.chunk_by_statistic(1.0, [](const std::vector<double>& chunk) {
        if (chunk.empty()) return 0.0;
        double sum = 0.0;
        for (size_t i = 1; i < chunk.size(); ++i) {
            sum += std::abs(chunk[i] - chunk[i-1]);
        }
        return sum / (chunk.size() - 1);
    });
    std::cout << "\nAdaptive size chunks (based on average difference):" << std::endl;
    print_chunks(adaptive_chunks);

    // Multi-criteria chunking
    std::vector<double> multi_data = {1.1, 1.2, 5.5, 5.6, 5.7, 2.1, 2.2, 7.5, 7.6, 7.7};
    Chunk<double> multi_chunker(1);
    multi_chunker.add(multi_data);
    
    // First by similarity
    auto multi_step1 = multi_chunker.chunk_by_similarity(0.5);
    std::cout << "\nMulti-criteria chunking - Step 1 (similarity):" << std::endl;
    print_chunks(multi_step1);
    
    // Then by size
    std::vector<std::vector<double>> multi_step2;
    for (const auto& chunk : multi_step1) {
        Chunk<double> size_chunker(2);
        size_chunker.add(chunk);
        auto sized_chunks = size_chunker.get_chunks();
        multi_step2.insert(multi_step2.end(), sized_chunks.begin(), sized_chunks.end());
    }
    std::cout << "Multi-criteria chunking - Step 2 (size):" << std::endl;
    print_chunks(multi_step2);

    // Sliding window with custom aggregation
    std::vector<double> sliding_data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    Chunk<double> sliding_chunker(3);
    sliding_chunker.add(sliding_data);
    
    auto sliding_chunks = sliding_chunker.sliding_window(3, 2);
    std::cout << "\nSliding window chunks with aggregation:" << std::endl;
    print_chunks(sliding_chunks);
    
    // Calculate moving averages
    std::vector<double> moving_averages;
    for (const auto& window : sliding_chunks) {
        double avg = std::accumulate(window.begin(), window.end(), 0.0) / window.size();
        moving_averages.push_back(avg);
    }
    std::cout << "Moving averages: ";
    for (double avg : moving_averages) {
        std::cout << std::fixed << std::setprecision(2) << avg << " ";
    }
    std::cout << std::endl;

    // Dynamic threshold chunking
    std::vector<double> dynamic_data = {1.0, 1.1, 1.2, 5.0, 5.1, 5.2, 2.0, 2.1, 2.2};
    Chunk<double> dynamic_chunker(1);
    dynamic_chunker.add(dynamic_data);
    
    double threshold = 0.5;
    auto dynamic_chunks = dynamic_chunker.chunk_by_predicate([&threshold](double x) {
        static double last = x;
        bool start_new = std::abs(x - last) > threshold;
        threshold = std::max(0.2, threshold * 0.95); // Gradually decrease threshold
        last = x;
        return start_new;
    });
    std::cout << "\nDynamic threshold chunks:" << std::endl;
    print_chunks(dynamic_chunks);

    return 0;
}