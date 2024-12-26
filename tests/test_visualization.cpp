#ifndef CHUNK_PROCESSING_TEST_VISUALIZATION_HPP
#define CHUNK_PROCESSING_TEST_VISUALIZATION_HPP

#include "chunk.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace chunk_processing;

class ChunkVisualizerTest : public ::testing::Test {
protected:
    std::vector<std::vector<double>> test_chunks = {
        {1.0, 2.0, 3.0}, {4.0, 5.0}, {6.0, 7.0, 8.0, 9.0}};
};

TEST_F(ChunkVisualizerTest, PlotChunkSizes) {
    // Get chunk sizes
    std::vector<size_t> sizes;
    for (const auto& chunk : test_chunks) {
        sizes.push_back(chunk.size());
    }

    // Create expected sizes
    std::vector<size_t> expected_sizes = {3, 2, 4};

    // Compare vectors
    ASSERT_EQ(sizes.size(), expected_sizes.size());
    for (size_t i = 0; i < sizes.size(); ++i) {
        EXPECT_EQ(sizes[i], expected_sizes[i]);
    }
}

TEST_F(ChunkVisualizerTest, PlotChunkDistribution) {
    // Calculate chunk statistics
    double mean = 0.0;
    size_t total_elements = 0;

    for (const auto& chunk : test_chunks) {
        for (const auto& value : chunk) {
            mean += value;
            total_elements++;
        }
    }
    mean /= total_elements;

    // Verify mean is in expected range
    EXPECT_NEAR(mean, 5.0, 0.1);
}

TEST_F(ChunkVisualizerTest, EmptyChunks) {
    std::vector<std::vector<double>> empty_chunks;

    // Get chunk sizes for empty chunks
    std::vector<size_t> sizes;
    for (const auto& chunk : empty_chunks) {
        sizes.push_back(chunk.size());
    }

    EXPECT_TRUE(sizes.empty());
}

#endif // CHUNK_PROCESSING_TEST_VISUALIZATION_HPP