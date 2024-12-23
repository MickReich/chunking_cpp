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
#include <memory>
#include <string>
#include <vector>

#ifdef __linux__
#include <unistd.h> // For getpagesize()
#endif

namespace chunk_benchmark {

/**
 * @brief Abstract base class for chunking strategies
 * @tparam T The data type of the chunks
 */
template <typename T>
class ChunkStrategy {
public:
    virtual ~ChunkStrategy() = default;

    /**
     * @brief Process data into chunks
     * @param data Input data to chunk
     * @return Vector of chunks
     */
    virtual std::vector<std::vector<T>> chunk(const std::vector<T>& data) = 0;

    /**
     * @brief Get strategy name
     * @return String identifier for the strategy
     */
    virtual std::string name() const = 0;
};

/**
 * @brief Class for benchmarking chunk processing performance
 * @tparam T The data type of the chunks
 */
template <typename T>
class ChunkBenchmark {
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
    void add_strategy(std::shared_ptr<ChunkStrategy<T>> strategy) {
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

            strategy->chunk(test_data);

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            results.emplace_back(strategy->name(), duration.count());
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
        strategies[0]->chunk(test_data);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        return static_cast<double>(test_data.size()) / duration.count();
    }

    /**
     * @brief Measure memory usage during processing
     * @return Peak memory usage in bytes
     */
    size_t measure_memory_usage() {
// Implementation depends on platform
#ifdef __linux__
        std::ifstream stat("/proc/self/statm");
        size_t resident;
        stat >> resident; // Skip first value
        stat >> resident; // Read resident set size
        return resident * getpagesize();
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

private:
    const std::vector<T>& test_data;                           ///< Reference to test data
    std::string results_dir;                                   ///< Directory for storing results
    std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies; ///< Registered strategies
    std::vector<std::pair<std::string, double>> results;       ///< Benchmark results
};

} // namespace chunk_benchmark