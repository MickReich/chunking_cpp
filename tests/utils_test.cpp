#include "utils.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace chunk_utils;

class StatisticsTest : public ::testing::Test {
protected:
    std::vector<double> test_data = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> empty_data = {};
    std::vector<double> duplicate_data = {1.0, 2.0, 2.0, 3.0, 4.0};
};

// Statistics Tests
TEST_F(StatisticsTest, MeanCalculation) {
    EXPECT_DOUBLE_EQ(Statistics<double>::mean(test_data), 3.0);
    EXPECT_DOUBLE_EQ(Statistics<double>::mean(empty_data), 0.0);
}

TEST_F(StatisticsTest, MedianCalculation) {
    EXPECT_DOUBLE_EQ(Statistics<double>::median(test_data), 3.0);
    EXPECT_DOUBLE_EQ(Statistics<double>::median(empty_data), 0.0);
    
    // Even number of elements
    std::vector<double> even_data = {1.0, 2.0, 3.0, 4.0};
    EXPECT_DOUBLE_EQ(Statistics<double>::median(even_data), 2.5);
}

TEST_F(StatisticsTest, ModeCalculation) {
    auto mode_result = Statistics<double>::mode(duplicate_data);
    EXPECT_DOUBLE_EQ(mode_result.first, 2.0);
    EXPECT_EQ(mode_result.second, 2);
    
    auto empty_mode = Statistics<double>::mode(empty_data);
    EXPECT_DOUBLE_EQ(empty_mode.first, 0.0);
    EXPECT_EQ(empty_mode.second, 0);
}

// ChunkManipulator Tests
class ChunkManipulatorTest : public ::testing::Test {
protected:
    std::vector<std::vector<double>> chunks1 = {{1.0, 2.0}, {3.0, 4.0}};
    std::vector<std::vector<double>> chunks2 = {{5.0, 6.0}, {7.0, 8.0}};
    std::vector<std::vector<double>> empty_chunks = {};
};

TEST_F(ChunkManipulatorTest, MergeChunks) {
    auto merged = ChunkManipulator<double>::merge_chunks(chunks1, chunks2);
    EXPECT_EQ(merged.size(), 4);
    EXPECT_EQ(merged[0], std::vector<double>({1.0, 2.0}));
    EXPECT_EQ(merged[3], std::vector<double>({7.0, 8.0}));
}

TEST_F(ChunkManipulatorTest, FilterChunks) {
    auto filtered = ChunkManipulator<double>::filter_chunks(
        chunks1, [](const std::vector<double>& chunk) { return chunk[0] > 2.0; });
    EXPECT_EQ(filtered.size(), 1);
    EXPECT_EQ(filtered[0], std::vector<double>({3.0, 4.0}));
}

TEST_F(ChunkManipulatorTest, TransformChunks) {
    auto transformed = ChunkManipulator<double>::transform_chunks(
        chunks1, [](const std::vector<double>& chunk) {
            std::vector<double> result = chunk;
            for (auto& val : result) val *= 2;
            return result;
        });
    EXPECT_EQ(transformed.size(), 2);
    EXPECT_EQ(transformed[0], std::vector<double>({2.0, 4.0}));
}

// ChunkGenerator Tests
class ChunkGeneratorTest : public ::testing::Test {
protected:
    const size_t test_size = 1000;
    const double min_val = 0.0;
    const double max_val = 10.0;
};

TEST_F(ChunkGeneratorTest, GenerateRandomData) {
    auto data = ChunkGenerator<double>::generate_random_data(test_size, min_val, max_val);
    EXPECT_EQ(data.size(), test_size);
    
    // Check bounds
    for (const auto& val : data) {
        EXPECT_GE(val, min_val);
        EXPECT_LE(val, max_val);
    }
}

TEST_F(ChunkGeneratorTest, GenerateRandomChunks) {
    const size_t num_chunks = 5;
    const size_t chunk_size = 10;
    
    auto chunks = ChunkGenerator<double>::generate_random_chunks(num_chunks, chunk_size, min_val, max_val);
    EXPECT_EQ(chunks.size(), num_chunks);
    
    for (const auto& chunk : chunks) {
        EXPECT_EQ(chunk.size(), chunk_size);
        for (const auto& val : chunk) {
            EXPECT_GE(val, min_val);
            EXPECT_LE(val, max_val);
        }
    }
}

// Edge Cases and Special Values
TEST_F(StatisticsTest, HandleSpecialValues) {
    std::vector<double> special_values = {
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::quiet_NaN()
    };
    
    // Mean should handle special values
    EXPECT_FALSE(std::isfinite(Statistics<double>::mean(special_values)));
}

TEST_F(ChunkManipulatorTest, EmptyChunkOperations) {
    // Merge with empty chunks
    auto merged_empty = ChunkManipulator<double>::merge_chunks(chunks1, empty_chunks);
    EXPECT_EQ(merged_empty.size(), chunks1.size());
    
    // Filter empty chunks
    auto filtered_empty = ChunkManipulator<double>::filter_chunks(
        empty_chunks, [](const std::vector<double>&) { return true; });
    EXPECT_TRUE(filtered_empty.empty());
    
    // Transform empty chunks
    auto transformed_empty = ChunkManipulator<double>::transform_chunks(
        empty_chunks, [](const std::vector<double>& chunk) { return chunk; });
    EXPECT_TRUE(transformed_empty.empty());
}

TEST_F(ChunkGeneratorTest, ZeroSizeGeneration) {
    auto empty_data = ChunkGenerator<double>::generate_random_data(0, min_val, max_val);
    EXPECT_TRUE(empty_data.empty());
    
    auto empty_chunks = ChunkGenerator<double>::generate_random_chunks(0, 5, min_val, max_val);
    EXPECT_TRUE(empty_chunks.empty());
}

// Performance Tests
TEST_F(ChunkGeneratorTest, LargeDataGeneration) {
    const size_t large_size = 1000000;
    auto large_data = ChunkGenerator<double>::generate_random_data(large_size, min_val, max_val);
    EXPECT_EQ(large_data.size(), large_size);
}