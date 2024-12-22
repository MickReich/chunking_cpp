#pragma once

#include <chrono>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <sys/resource.h>

namespace chunk_benchmark {

template<typename T>
class ChunkStrategy {
public:
    virtual std::vector<std::vector<T>> chunk(const std::vector<T>& data) = 0;
    virtual std::string name() const = 0;
    virtual ~ChunkStrategy() = default;
};

template<typename T>
class ChunkBenchmark {
private:
    const std::vector<T>& data;
    std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies;
    std::string output_dir;

    // Helper to get current memory usage in bytes
    size_t get_current_memory_usage() const {
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        return usage.ru_maxrss * 1024; // Convert to bytes
    }

    // Helper to format time duration
    std::string format_duration(std::chrono::nanoseconds ns) const {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
        return std::to_string(ms.count()) + "ms";
    }

    struct BenchmarkResults {
        std::unordered_map<std::string, double> throughput_metrics;
        std::unordered_map<std::string, size_t> memory_metrics;
        std::unordered_map<std::string, size_t> chunk_count_metrics;
        std::unordered_map<std::string, double> avg_chunk_size_metrics;
        std::string best_throughput_strategy;
        std::string best_memory_strategy;
    };

    BenchmarkResults results;

public:
    ChunkBenchmark(const std::vector<T>& input_data, const std::string& output_path = "./benchmark")
        : data(input_data), output_dir(output_path) {
        try {
            // Remove any existing file with the same name
            std::filesystem::remove(output_dir);
            
            // Create the directory and all parent directories
            std::filesystem::path dir(output_dir);
            if (!std::filesystem::exists(dir)) {
                if (!std::filesystem::create_directories(dir)) {
                    throw std::runtime_error("Failed to create directory: " + output_dir);
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << "\n";
            std::cerr << "Using current directory instead.\n";
            output_dir = ".";
        }
    }

    void add_strategy(std::shared_ptr<ChunkStrategy<T>> strategy) {
        strategies.push_back(strategy);
    }

    /**
     * @brief Run all benchmark measurements
     */
    void run_benchmark() {
        std::cout << "Starting benchmark comparison...\n\n";
        
        // Measure throughput
        std::cout << "Measuring throughput...\n";
        measure_throughput();
        
        // Measure memory usage
        std::cout << "Measuring memory usage...\n";
        measure_memory_usage();
        
        // Compare strategies
        std::cout << "Comparing strategies...\n";
        compare_strategies();
        
        std::cout << "\nBenchmark completed. Results will be saved to: " 
                  << output_dir << "\n";
    }

    /**
     * @brief Save benchmark results to files
     */
    void save_results() {
        try {
            // Create summary report
            auto summary_path = std::filesystem::path(output_dir) / "benchmark_summary.txt";
            std::ofstream summary(summary_path);
            if (!summary.is_open()) {
                throw std::runtime_error("Could not open summary file: " + summary_path.string());
            }

            summary << "Benchmark Summary Report\n"
                   << "======================\n\n";

            // Write throughput results
            summary << "Throughput Results:\n";
            summary << "-----------------\n";
            for (const auto& [strategy, throughput] : results.throughput_metrics) {
                summary << strategy << ": " << std::fixed << std::setprecision(2)
                       << throughput << " elements/second\n";
            }
            summary << "\n";

            // Write memory results
            summary << "Memory Usage Results:\n";
            summary << "-------------------\n";
            for (const auto& [strategy, memory] : results.memory_metrics) {
                summary << strategy << ": " << (memory / 1024.0) << " KB\n";
            }
            summary << "\n";

            // Write chunk statistics
            summary << "Chunk Statistics:\n";
            summary << "----------------\n";
            for (const auto& [strategy, count] : results.chunk_count_metrics) {
                summary << strategy << ":\n"
                       << "  - Chunk count: " << count << "\n"
                       << "  - Average chunk size: " << results.avg_chunk_size_metrics[strategy] << "\n";
            }
            summary << "\n";

            // Write best performers
            summary << "Best Performers:\n";
            summary << "--------------\n";
            summary << "Best throughput: " << results.best_throughput_strategy << "\n";
            summary << "Best memory usage: " << results.best_memory_strategy << "\n";

            std::cout << "Results have been saved to:\n"
                     << "- " << summary_path.string() << "\n"
                     << "- " << output_dir << "/throughput_report.txt\n"
                     << "- " << output_dir << "/memory_report.txt\n"
                     << "- " << output_dir << "/comparison_report.txt\n";

        } catch (const std::exception& e) {
            std::cerr << "Error saving results: " << e.what() << "\n";
            throw;
        }
    }

    void measure_throughput() {
        auto report_path = std::filesystem::path(output_dir) / "throughput_report.txt";
        std::ofstream report(report_path);
        if (!report.is_open()) {
            throw std::runtime_error("Could not open report file: " + report_path.string());
        }
        report << "Throughput Benchmark Report\n";
        report << "=========================\n\n";

        for (const auto& strategy : strategies) {
            // Warm-up run
            strategy->chunk(data);

            // Actual measurement
            auto start = std::chrono::high_resolution_clock::now();
            
            const int NUM_ITERATIONS = 10;
            for (int i = 0; i < NUM_ITERATIONS; ++i) {
                strategy->chunk(data);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>
                          (end - start) / NUM_ITERATIONS;

            double elements_per_second = (data.size() * 1e9) / duration.count();
            
            report << "Strategy: " << strategy->name() << "\n"
                  << "Average time: " << format_duration(duration) << "\n"
                  << "Throughput: " << std::fixed << std::setprecision(2) 
                  << elements_per_second << " elements/second\n\n";
            
            // Store results
            results.throughput_metrics[strategy->name()] = elements_per_second;
        }
    }

    void measure_memory_usage() {
        auto report_path = std::filesystem::path(output_dir) / "memory_report.txt";
        std::ofstream report(report_path);
        report << "Memory Usage Benchmark Report\n";
        report << "===========================\n\n";

        for (const auto& strategy : strategies) {
            size_t initial_memory = get_current_memory_usage();
            
            // Perform chunking
            auto chunks = strategy->chunk(data);
            
            size_t peak_memory = get_current_memory_usage();
            size_t memory_used = peak_memory - initial_memory;

            // Calculate chunk statistics
            size_t total_chunk_size = 0;
            size_t max_chunk_size = 0;
            for (const auto& chunk : chunks) {
                total_chunk_size += chunk.size();
                max_chunk_size = std::max(max_chunk_size, chunk.size());
            }

            report << "Strategy: " << strategy->name() << "\n"
                  << "Memory used: " << (memory_used / 1024.0) << " KB\n"
                  << "Number of chunks: " << chunks.size() << "\n"
                  << "Average chunk size: " << (total_chunk_size / chunks.size()) << "\n"
                  << "Max chunk size: " << max_chunk_size << "\n\n";
            
            // Store results
            results.memory_metrics[strategy->name()] = memory_used;
            results.chunk_count_metrics[strategy->name()] = chunks.size();
            results.avg_chunk_size_metrics[strategy->name()] = 
                static_cast<double>(total_chunk_size) / chunks.size();
        }
    }

    void compare_strategies() {
        auto report_path = std::filesystem::path(output_dir) / "comparison_report.txt";
        std::ofstream report(report_path);
        report << "Strategy Comparison Report\n";
        report << "========================\n\n";

        struct MetricData {
            double throughput;
            size_t memory_usage;
            size_t num_chunks;
            double avg_chunk_size;
        };

        std::unordered_map<std::string, MetricData> metrics;

        // Collect metrics for each strategy
        for (const auto& strategy : strategies) {
            // Measure throughput
            auto start = std::chrono::high_resolution_clock::now();
            auto chunks = strategy->chunk(data);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

            // Calculate metrics
            double throughput = (data.size() * 1e9) / duration.count();
            size_t memory_used = get_current_memory_usage();
            size_t total_size = 0;
            for (const auto& chunk : chunks) {
                total_size += chunk.size();
            }

            metrics[strategy->name()] = MetricData{
                throughput,
                memory_used,
                chunks.size(),
                static_cast<double>(total_size) / chunks.size()
            };
        }

        // Find best performers
        auto best_throughput = std::max_element(metrics.begin(), metrics.end(),
            [](const auto& a, const auto& b) { return a.second.throughput < b.second.throughput; });
        auto best_memory = std::min_element(metrics.begin(), metrics.end(),
            [](const auto& a, const auto& b) { return a.second.memory_usage < b.second.memory_usage; });

        // Write comparison report
        report << "Performance Summary:\n\n";
        for (const auto& [name, metric] : metrics) {
            report << "Strategy: " << name << "\n"
                  << "- Throughput: " << std::fixed << std::setprecision(2) 
                  << metric.throughput << " elements/second\n"
                  << "- Memory usage: " << (metric.memory_usage / 1024.0) << " KB\n"
                  << "- Number of chunks: " << metric.num_chunks << "\n"
                  << "- Average chunk size: " << metric.avg_chunk_size << "\n\n";
        }

        report << "\nBest Performers:\n"
               << "- Highest throughput: " << best_throughput->first << "\n"
               << "- Lowest memory usage: " << best_memory->first << "\n";
        
        // Store best performers
        results.best_throughput_strategy = best_throughput->first;
        results.best_memory_strategy = best_memory->first;
    }
};

} // namespace chunk_benchmark 