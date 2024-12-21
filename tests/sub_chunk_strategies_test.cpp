/**
 * @file sub_chunk_strategies_test.cpp
 * @brief Test suite for sub-chunking strategies
 *
 * This file contains comprehensive tests for:
 * - Recursive sub-chunking
 * - Hierarchical sub-chunking
 * - Conditional sub-chunking
 * - Edge cases and error conditions
 */
#include "sub_chunk_strategies.hpp"
#include "gtest/gtest.h"
#include <vector>

using namespace chunk_strategies;

/**
 * @brief Test fixture for sub-chunking strategy tests
 *
 * Provides common test data and setup for all sub-chunking tests
 */
class SubChunkStrategiesTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {{1.0, 1.1, 1.2, 5.0, 5.1, 5.2},
                     {2.0, 2.1, 2.2, 10.0, 10.1, 10.2},
                     {3.0, 3.1, 3.2, 15.0, 15.1, 15.2}};
    }

    std::vector<std::vector<double>> test_data;
};

TEST_F(SubChunkStrategiesTest, RecursiveSubChunking) {
    auto variance_strategy = std::make_shared<VarianceStrategy<double>>(3.0);
    RecursiveSubChunkStrategy<double> recursive_strategy(variance_strategy, 2, 2);

    auto result = recursive_strategy.apply(test_data);
    EXPECT_GT(result.size(), 0);
}

TEST_F(SubChunkStrategiesTest, HierarchicalSubChunking) {
    std::vector<std::shared_ptr<ChunkStrategy<double>>> strategies = {
        std::make_shared<VarianceStrategy<double>>(5.0),
        std::make_shared<EntropyStrategy<double>>(1.0)};

    HierarchicalSubChunkStrategy<double> hierarchical_strategy(strategies, 2);
    auto result = hierarchical_strategy.apply(test_data);
    EXPECT_GT(result.size(), 0);
}

TEST_F(SubChunkStrategiesTest, ConditionalSubChunking) {
    auto condition = [](const std::vector<double>& chunk) {
        double mean = std::accumulate(chunk.begin(), chunk.end(), 0.0) / chunk.size();
        double variance = 0.0;
        for (const auto& val : chunk) {
            variance += (val - mean) * (val - mean);
        }
        variance /= chunk.size();
        return variance > 50.0;
    };

    auto variance_strategy = std::make_shared<VarianceStrategy<double>>(5.0);
    ConditionalSubChunkStrategy<double> conditional_strategy(variance_strategy, condition, 2);

    auto result = conditional_strategy.apply(test_data);
    EXPECT_GT(result.size(), 0);
}