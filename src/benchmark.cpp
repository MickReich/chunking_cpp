/**
 * @file benchmark.cpp
 * @brief Implementation of benchmarking utilities
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#include "chunk.hpp"
#include "chunk_benchmark.hpp"
#include "chunk_strategies.hpp"
#include "chunk_strategy_implementations.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

template <typename T>
void run_benchmark(const std::vector<T>& data) {
    chunk_benchmark::ChunkBenchmark<T> benchmark(data);

    // Add different chunking strategies
    benchmark.add_strategy(std::make_shared<chunk_processing::NeuralChunkingStrategy<T>>());
    benchmark.add_strategy(std::make_shared<chunk_processing::SimilarityChunkingStrategy<T>>(0.5));

    // Run benchmark and print results
    auto results = benchmark.benchmark_chunking();
    for (const auto& result : results) {
        std::cout << "Strategy: " << result.strategy_name << "\n"
                  << "  Execution time: " << result.execution_time_ms << " ms\n"
                  << "  Memory usage: " << result.memory_usage_bytes << " bytes\n"
                  << "  Average chunks: " << result.num_chunks << "\n\n";
    }
}

int main() {
    // Example data for benchmarking
    std::vector<int> data(1000);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<int>(i % 10);
    }

    std::cout << "Running benchmark with integer data...\n";
    run_benchmark(data);

    return 0;
}