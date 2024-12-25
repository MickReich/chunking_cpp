#include "chunk_strategies.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <vector>

class AdvancedChunkStrategiesTest : public ::testing::Test {
protected:
    std::vector<double> test_data = {1.0, 2.0, 3.0, 1.0, 2.0, 3.0};
    std::vector<int> cyclic_data = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    std::vector<double> single_element = {1.0};
    std::vector<double> empty_data;
    std::vector<std::vector<double>> multi_dim_data = {
        {1.0, 2.0}, {2.0, 3.0}, {3.0, 4.0}, {1.0, 2.0}, {2.0, 3.0}};
};

TEST_F(AdvancedChunkStrategiesTest, PatternBasedPredicate) {
    auto predicate = [](double val) { return val > 2.0; };
    chunk_processing::PatternBasedStrategy<double> strategy(predicate);

    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 0);
}

TEST_F(AdvancedChunkStrategiesTest, PatternBasedCyclicDetection) {
    chunk_processing::PatternBasedStrategy<int> strategy(3); // Pattern size of 3
    auto chunks = strategy.apply(cyclic_data);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_EQ(chunk.size(), 3);
    }
}

TEST_F(AdvancedChunkStrategiesTest, SingleElementInput) {
    // Test with pattern size
    chunk_processing::PatternBasedStrategy<double> size_strategy(2);
    auto size_chunks = size_strategy.apply(single_element);
    EXPECT_EQ(size_chunks.size(), 1);
    EXPECT_EQ(size_chunks[0].size(), 1);

    // Test with predicate
    chunk_processing::PatternBasedStrategy<double> pred_strategy([](double) { return true; });
    auto pred_chunks = pred_strategy.apply(single_element);
    EXPECT_EQ(pred_chunks.size(), 1);
    EXPECT_EQ(pred_chunks[0].size(), 1);
}

TEST_F(AdvancedChunkStrategiesTest, EmptyInput) {
    // Test with pattern size
    chunk_processing::PatternBasedStrategy<double> size_strategy(2);
    auto size_chunks = size_strategy.apply(empty_data);
    EXPECT_TRUE(size_chunks.empty());

    // Test with predicate
    chunk_processing::PatternBasedStrategy<double> pred_strategy([](double) { return true; });
    auto pred_chunks = pred_strategy.apply(empty_data);
    EXPECT_TRUE(pred_chunks.empty());
}

TEST_F(AdvancedChunkStrategiesTest, MultiDimensionalThresholds) {
    auto vector_distance = [](const std::vector<double>& a, const std::vector<double>& b) {
        double sum = 0.0;
        for (size_t i = 0; i < a.size(); ++i) {
            sum += (a[i] - b[i]) * (a[i] - b[i]);
        }
        return std::sqrt(sum);
    };

    auto predicate = [vector_distance](const std::vector<double>& vec) {
        return vector_distance(vec, {0.0, 0.0}) > 2.0;
    };

    chunk_processing::PatternBasedStrategy<std::vector<double>> strategy(predicate);
    auto chunks = strategy.apply(multi_dim_data);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
        for (const auto& vec : chunk) {
            EXPECT_EQ(vec.size(), 2);
        }
    }
}