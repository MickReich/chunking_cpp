#include "../include/chunk_benchmark.hpp"
#include "../include/neural_chunking.hpp"
#include "../include/chunk.hpp"
#include <random>
#include <memory>

// Strategy wrapper for neural chunking
template<typename T>
class NeuralChunkingStrategy : public chunk_benchmark::ChunkStrategy {
private:
    neural_chunking::NeuralChunking<T> chunker;

public:
    NeuralChunkingStrategy() : chunker(8, 0.5) {}
    
    std::vector<std::vector<T>> chunk(const std::vector<T>& data) override {
        return chunker.chunk(data);
    }
    
    std::string name() const override {
        return "Neural Chunking";
    }
};

// Strategy wrapper for similarity-based chunking
template<typename T>
class SimilarityChunkingStrategy : public chunk_benchmark::ChunkStrategy {
private:
    Chunk<T> chunker;
    T threshold;

public:
    SimilarityChunkingStrategy(T thresh) : chunker(10), threshold(thresh) {}
    
    std::vector<std::vector<T>> chunk(const std::vector<T>& data) override {
        chunker.add(data);
        return chunker.chunk_by_similarity(threshold);
    }
    
    std::string name() const override {
        return "Similarity-based Chunking";
    }
};

// Generate sample data with patterns
std::vector<int> generate_test_data(size_t size) {
    std::vector<int> data;
    data.reserve(size);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(50, 15);  // mean of 50, std dev of 15
    
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

int main() {
    try {
        // Use the generate_test_data instead of the simple test data for better testing
        std::vector<int> test_data = generate_test_data(1000);
        chunk_benchmark::ChunkBenchmark<int> benchmark(test_data, "./benchmark_results");
        
        // Add strategies and run benchmarks
        auto neural_strategy = std::make_shared<NeuralChunkingStrategy<int>>();
        auto similarity_strategy = std::make_shared<SimilarityChunkingStrategy<int>>(0.3);
        
        benchmark.add_strategy(neural_strategy);
        benchmark.add_strategy(similarity_strategy);
        
        // Run the benchmark and output results
        benchmark.run_benchmark();
        benchmark.save_results();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}