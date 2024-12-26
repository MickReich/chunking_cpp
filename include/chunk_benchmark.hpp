/**
 * @file chunk_benchmark.hpp
 * @brief Performance benchmarking tools for chunk processing
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once

#include "chunk_common.hpp"
#include "chunk_strategies.hpp"
#include <chrono>
#include <string>
#include <vector>

namespace chunk_benchmark {

struct BenchmarkResult {
    double execution_time_ms;
    size_t memory_usage_bytes;
    size_t num_chunks;
    std::string strategy_name;
};

template <typename T>
class ChunkBenchmark {
private:
    std::vector<T> test_data;
    std::vector<std::shared_ptr<chunk_processing::ChunkStrategy<T>>> strategies;
    size_t iterations;

public:
    explicit ChunkBenchmark(const std::vector<T>& data, size_t num_iterations = 100)
        : test_data(data), iterations(num_iterations) {}

    void add_strategy(std::shared_ptr<chunk_processing::ChunkStrategy<T>> strategy) {
        strategies.push_back(strategy);
    }

    std::vector<BenchmarkResult> benchmark_chunking() {
        std::vector<BenchmarkResult> results;
        for (const auto& strategy : strategies) {
            auto start = std::chrono::high_resolution_clock::now();

            size_t total_chunks = 0;
            for (size_t i = 0; i < iterations; ++i) {
                auto chunks = strategy->apply(test_data);
                total_chunks += chunks.size();
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            results.push_back({static_cast<double>(duration.count()), sizeof(T) * test_data.size(),
                               total_chunks / iterations, "Strategy"});
        }
        return results;
    }

    void save_results(const std::string& filename) const {
        // Implementation for saving results to file
    }
};

} // namespace chunk_benchmark