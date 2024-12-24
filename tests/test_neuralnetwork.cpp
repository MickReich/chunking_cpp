#include "neural_chunking.hpp"
#include <gtest/gtest.h>

class NeuralNetworkTest : public ::testing::Test {
protected:
    neural_chunking::NeuralChunking<double> chunker{3, 0.5};
    std::vector<double> test_data{1.0, 2.0, 3.0, 5.0, 6.0, 1.0};
};

TEST_F(NeuralNetworkTest, BasicChunking) {
    auto chunks = chunker.chunk(test_data);
    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_FALSE(chunk.empty());
    }
}

TEST_F(NeuralNetworkTest, EmptyInput) {
    std::vector<double> empty_data;
    auto chunks = chunker.chunk(empty_data);
    EXPECT_TRUE(chunks.empty());
}

TEST_F(NeuralNetworkTest, SmallInput) {
    std::vector<double> small_data{1.0, 2.0}; // Smaller than window size
    auto chunks = chunker.chunk(small_data);
    EXPECT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0], small_data);
}

TEST_F(NeuralNetworkTest, Configuration) {
    EXPECT_EQ(chunker.get_window_size(), 3);
    EXPECT_DOUBLE_EQ(chunker.get_threshold(), 0.5);

    chunker.set_window_size(4);
    chunker.set_threshold(0.7);

    EXPECT_EQ(chunker.get_window_size(), 4);
    EXPECT_DOUBLE_EQ(chunker.get_threshold(), 0.7);
}

TEST_F(NeuralNetworkTest, ChunkBoundaries) {
    std::vector<double> data{1.0, 1.0, 1.0, 5.0, 5.0, 5.0};
    auto chunks = chunker.chunk(data);
    EXPECT_GT(chunks.size(), 1); // Should detect the boundary between 1s and 5s
}
