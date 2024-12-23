/**
 * @file benchmark.cpp
 * @brief Implementation of benchmarking utilities
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#include "../include/chunk.hpp"
#include "../include/chunk_benchmark.hpp"
#include "../include/neural_chunking.hpp"
#include <memory>
#include <random>

// Strategy wrapper for neural chunking
template <typename T>
class NeuralChunkingStrategy : public chunk_benchmark::ChunkStrategy<T> {
private:
    neural_chunking::NeuralChunking<T> neural_chunker;

public:
    NeuralChunkingStrategy() : neural_chunker(8, 0.5) {}

    std::vector<std::vector<T>> chunk(const std::vector<T>& data) override {
        return neural_chunker.chunk(data);
    }

    std::string name() const override {
        return "Neural Chunking";
    }
};

// Strategy wrapper for similarity-based chunking
template <typename T>
class SimilarityChunkingStrategy : public chunk_benchmark::ChunkStrategy<T> {
private:
    double threshold;

public:
    explicit SimilarityChunkingStrategy(double t) : threshold(t) {}

    std::vector<std::vector<T>> chunk(const std::vector<T>& data) override {
        Chunk<T> chunker(data.size());
        for (const auto& item : data) {
            chunker.add(item);
        }
        return chunker.chunk_by_similarity(threshold);
    }

    std::string name() const override {
        return "Similarity Chunking";
    }
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

    benchmark.add_strategy(std::make_shared<NeuralChunkingStrategy<T>>());
    benchmark.add_strategy(std::make_shared<SimilarityChunkingStrategy<T>>(0.5));

    auto results = benchmark.run_benchmark();
    benchmark.save_results();
}

int main() {
    std::vector<int> data = generate_test_data(1000);
    run_benchmark<int>(data);
    return 0;
}