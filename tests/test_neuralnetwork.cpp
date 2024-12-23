#include "neural_chunking.hpp"
#include <gtest/gtest.h>
#include <vector>

class NeuralChunkingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize with default values
        chunker = std::make_unique<neural_chunking::NeuralChunking<int>>(4, 0.5);
        test_data = {1, 2, 3, 4, 5, 6, 7, 8};
    }

    std::unique_ptr<neural_chunking::NeuralChunking<int>> chunker;
    std::vector<int> test_data;
};

TEST_F(NeuralChunkingTest, LayerInitialization) {
    neural_chunking::Layer<double> layer(4, 2);
    std::vector<double> input = {1.0, 2.0, 3.0, 4.0};
    auto output = layer.forward(input);
    EXPECT_EQ(output.size(), 2);
}

TEST_F(NeuralChunkingTest, InvalidInput) {
    neural_chunking::Layer<double> layer(4, 2);
    std::vector<double> invalid_input = {1.0, 2.0}; // Wrong size
    EXPECT_THROW(layer.forward(invalid_input), std::invalid_argument);
}

TEST_F(NeuralChunkingTest, BasicChunking) {
    auto chunks = chunker->chunk(test_data);
    EXPECT_GT(chunks.size(), 0);
}

TEST_F(NeuralChunkingTest, EmptyInput) {
    std::vector<int> empty_data;
    auto chunks = chunker->chunk(empty_data);
    EXPECT_EQ(chunks.size(), 1);
    EXPECT_TRUE(chunks[0].empty());
}

TEST_F(NeuralChunkingTest, SmallInput) {
    std::vector<int> small_data = {1, 2};
    auto chunks = chunker->chunk(small_data);
    EXPECT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0], small_data);
}

TEST_F(NeuralChunkingTest, DifferentDataTypes) {
    neural_chunking::NeuralChunking<double> double_chunker(4, 0.5);
    std::vector<double> double_data = {1.0, 2.0, 3.0, 4.0, 5.0};
    auto chunks = double_chunker.chunk(double_data);
    EXPECT_GT(chunks.size(), 0);
}
