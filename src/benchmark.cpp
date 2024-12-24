/**
 * @file benchmark.cpp
 * @brief Implementation of benchmarking utilities
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#include "chunk.hpp"
#include "chunk_benchmark.hpp"
#include "chunk_strategies.hpp"
#include "neural_chunking.hpp"
#include <iostream>
#include <memory>
#include <random>

// Strategy wrapper for neural chunking
template <typename T>
class NeuralChunkingStrategy : public chunk_strategies::ChunkStrategy<T> {
private:
    mutable neural_chunking::NeuralChunking<T> neural_chunker;

public:
    NeuralChunkingStrategy() : neural_chunker(8, 0.5) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        return neural_chunker.chunk(data);
    }
};

// Strategy wrapper for similarity-based chunking
template <typename T>
class SimilarityChunkingStrategy : public chunk_strategies::ChunkStrategy<T> {
public:
    explicit SimilarityChunkingStrategy(double threshold) : threshold(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        chunk_processing::Chunk<T> chunker(1); // Start with smallest chunk size
        for (const auto& item : data) {
            chunker.add(item);
        }
        return chunker.chunk_by_threshold(threshold);
    }

private:
    double threshold;
};

// Generate sample data with patterns
std::vector<int> generate_test_data(size_t size) {
    std::vector<int> data;
    data.reserve(size);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(50, 15); // mean of 50, std dev of 15

    // Generate data with some patterns and noise
    for (size_t i = 0; i < size; ++i) {
        if (i % 20 < 10) {
            // Create a pattern
            data.push_back(static_cast<int>(i % 10));
        } else {
            // Add some noise
            data.push_back(static_cast<int>(d(gen)));
        }
    }

    return data;
}

// Update the benchmark function to use templated strategies
template <typename T>
void run_benchmark(const std::vector<T>& data) {
    chunk_benchmark::ChunkBenchmark<T> benchmark(data, "./benchmark_results");

    // Add strategies
    auto neural_strategy = std::make_shared<NeuralChunkingStrategy<T>>();
    auto similarity_strategy = std::make_shared<SimilarityChunkingStrategy<T>>(0.5);

    benchmark.add_strategy(neural_strategy);
    benchmark.add_strategy(similarity_strategy);

    // Run benchmarks
    auto results = benchmark.run_benchmark();
    benchmark.save_results();

    // Test specific chunking operation
    auto chunk_result = benchmark.benchmark_chunking(data, 64);
    std::cout << "Chunking time: " << chunk_result.execution_time_ms << "ms\n";
    std::cout << "Memory usage: " << chunk_result.memory_usage_bytes << " bytes\n";
}

int main() {
    std::vector<int> data = generate_test_data(1000);
    run_benchmark<int>(data);
    return 0;
}