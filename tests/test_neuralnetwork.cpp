#include <gtest/gtest.h>
#include "../include/neural_chunking.hpp"
#include <vector>
#include <stdexcept>

class NeuralChunkingTest : public ::testing::Test {
protected:
    neural_chunking::NeuralChunking<int> chunker;
    std::vector<int> sample_data;

    void SetUp() override {
        // Create sample data
        for (int i = 0; i < 20; ++i) {
            sample_data.push_back(i % 5);
        }
    }
};

// Test Layer class
TEST_F(NeuralChunkingTest, LayerInitialization) {
    neural_chunking::Layer layer(4, 2);
    std::vector<double> input = {0.1, 0.2, 0.3, 0.4};
    auto output = layer.forward(input);
    
    EXPECT_EQ(output.size(), 2);
    for (const auto& val : output) {
        EXPECT_GE(val, 0.0);
        EXPECT_LE(val, 1.0);
    }
}

TEST_F(NeuralChunkingTest, LayerInputValidation) {
    neural_chunking::Layer layer(4, 2);
    std::vector<double> invalid_input = {0.1, 0.2, 0.3}; // Wrong size
    
    EXPECT_THROW(layer.forward(invalid_input), std::invalid_argument);
}

// Test NeuralChunking class
TEST_F(NeuralChunkingTest, Constructor) {
    neural_chunking::NeuralChunking<int> custom_chunker(10, 0.7);
    EXPECT_EQ(custom_chunker.get_window_size(), 10);
}

TEST_F(NeuralChunkingTest, ThresholdValidation) {
    EXPECT_THROW(chunker.set_threshold(-0.1), std::invalid_argument);
    EXPECT_THROW(chunker.set_threshold(1.1), std::invalid_argument);
    EXPECT_NO_THROW(chunker.set_threshold(0.5));
}

TEST_F(NeuralChunkingTest, SmallDataHandling) {
    std::vector<int> small_data = {1, 2, 3};
    auto chunks = chunker.chunk(small_data);
    
    EXPECT_GE(chunks.size(), 1);
    size_t total_elements = 0;
    for (const auto& chunk : chunks) {
        total_elements += chunk.size();
    }
    EXPECT_EQ(total_elements, small_data.size());
}

TEST_F(NeuralChunkingTest, ChunkConsistency) {
    auto chunks = chunker.chunk(sample_data);
    
    // Verify that all elements are preserved
    size_t total_elements = 0;
    for (const auto& chunk : chunks) {
        EXPECT_FALSE(chunk.empty());
        total_elements += chunk.size();
    }
    EXPECT_EQ(total_elements, sample_data.size());
}

TEST_F(NeuralChunkingTest, DifferentThresholds) {
    // Test with different thresholds
    chunker.set_threshold(0.3);
    auto chunks_loose = chunker.chunk(sample_data);
    
    chunker.set_threshold(0.7);
    auto chunks_strict = chunker.chunk(sample_data);
    
    // Higher threshold should result in fewer or equal number of chunks
    EXPECT_LE(chunks_strict.size(), chunks_loose.size());
}

TEST_F(NeuralChunkingTest, EmptyInput) {
    std::vector<int> empty_data;
    auto chunks = chunker.chunk(empty_data);
    EXPECT_TRUE(chunks.empty());
}

// Test with different data types
TEST_F(NeuralChunkingTest, DifferentDataTypes) {
    neural_chunking::NeuralChunking<double> double_chunker;
    std::vector<double> double_data = {1.1, 2.2, 3.3, 4.4, 5.5};
    auto chunks = double_chunker.chunk(double_data);
    
    EXPECT_FALSE(chunks.empty());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 