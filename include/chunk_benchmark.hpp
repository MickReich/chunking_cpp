/**
 * @file chunk_benchmark.hpp
 * @brief Performance benchmarking tools for chunk processing
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <typeinfo>
#ifdef __linux__
#include <unistd.h> // For getpagesize()
#endif
#include "chunk_common.hpp"
#include "chunk_strategies.hpp"
#include "chunk.hpp"

namespace chunk_benchmark {

struct BenchmarkResult {
    double execution_time_ms;
    size_t memory_usage_bytes;
    size_t num_chunks;
    std::string strategy_name;
};

/**
 * @brief Abstract base class for chunking strategies
 * @tparam T The data type of the chunks
 */
template <typename T>
class CHUNK_EXPORT ChunkStrategy {
public:
    virtual ~ChunkStrategy() = default;
    virtual std::vector<std::vector<T>> apply(const std::vector<T>& data) const = 0;
};

/**
 * @brief Class for benchmarking chunk processing performance
 * @tparam T The data type of the chunks
 */
template <typename T>
class CHUNK_EXPORT ChunkBenchmark {
public:
    /**
     * @brief Constructor
     * @param data Reference to test data
     * @param output_dir Directory for benchmark results
     */
    ChunkBenchmark(const std::vector<T>& data,
                   const std::string& output_dir = "./benchmark_results")
        : test_data(data), results_dir(output_dir) {
        std::filesystem::create_directories(results_dir);
    }

    /**
     * @brief Add a chunking strategy to benchmark
     * @param strategy Strategy to test
     */
    void add_strategy(std::shared_ptr<chunk_strategies::ChunkStrategy<T>> strategy) {
        strategies.push_back(strategy);
    }

    /**
     * @brief Run complete benchmark suite
     * @return Vector of benchmark results
     */
    std::vector<std::pair<std::string, double>> run_benchmark() {
        results.clear();
        for (const auto& strategy : strategies) {
            auto start = std::chrono::high_resolution_clock::now();
            strategy->apply(test_data);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            results.emplace_back(typeid(*strategy).name(), duration.count());
        }
        return results;
    }

    /**
     * @brief Measure processing throughput
     * @return Throughput in elements per second
     */
    double measure_throughput() {
        if (strategies.empty())
            return 0.0;

        auto start = std::chrono::high_resolution_clock::now();
        strategies[0]->apply(test_data);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        return static_cast<double>(test_data.size()) / duration.count();
    }

    /**
     * @brief Measure memory usage during processing
     * @return Peak memory usage in bytes
     */
    size_t measure_memory_usage() {
    #ifdef __linux__
        std::ifstream stat("/proc/self/statm");
        size_t resident;
        stat >> resident; // Skip first value
        stat >> resident; // Read resident set size
        return resident * static_cast<size_t>(sysconf(_SC_PAGESIZE));
    #else
        return 0; // Placeholder for other platforms
    #endif
    }

    /**
     * @brief Compare different chunking strategies
     * @return Vector of strategy comparison results
     */
    std::vector<std::pair<std::string, double>> compare_strategies() {
        return run_benchmark();
    }

    /**
     * @brief Save benchmark results to file
     */
    void save_results() {
        std::ofstream file(results_dir + "/benchmark_results.txt");
        file << "Strategy,Time (ms)\n";
        for (const auto& result : results) {
            file << result.first << "," << result.second << "\n";
        }
    }

    BenchmarkResult benchmark_chunking(const std::vector<T>& data, size_t chunk_size) {
        auto start = std::chrono::high_resolution_clock::now();
        
        chunk_processing::Chunk<T> chunker(chunk_size);
        chunker.add(data);
        auto chunks = chunker.get_chunks();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        return BenchmarkResult{
            .execution_time_ms = static_cast<double>(duration.count()),
            .memory_usage_bytes = measure_memory_usage(),
            .num_chunks = chunks.size(),
            .strategy_name = "basic_chunking"
        };
    }

private:
    const std::vector<T>& test_data;                           ///< Reference to test data
    std::string results_dir;                                   ///< Directory for storing results
    std::vector<std::shared_ptr<chunk_strategies::ChunkStrategy<T>>> strategies; ///< Registered strategies
    std::vector<std::pair<std::string, double>> results;       ///< Benchmark results
};

} // namespace chunk_benchmark