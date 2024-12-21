#include "chunk_strategies.hpp"
#include "gtest/gtest.h"
#include <vector>

using namespace chunk_strategies;

class QuantileStrategyTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1.0, 2.0, 5.0, 6.0, 3.0, 4.0, 8.0, 7.0};
    }
    std::vector<double> test_data;
};

class VarianceStrategyTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1.0, 1.1, 1.2, 5.0, 5.1, 5.2, 2.0, 2.1, 2.2};
    }
    std::vector<double> test_data;
};

class EntropyStrategyTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1, 1, 1, 2, 2, 3, 4, 4, 4, 4};
    }
    std::vector<double> test_data;
};

TEST_F(QuantileStrategyTest, BasicOperation) {
    QuantileStrategy<double> strategy(0.5); // median

    auto chunks = strategy.apply(test_data);
    EXPECT_EQ(chunks.size(), 2); // Should split into two chunks at median

    // Get the median value
    std::vector<double> sorted_data = test_data;
    std::sort(sorted_data.begin(), sorted_data.end());
    double median = sorted_data[sorted_data.size() / 2];

    // Verify that values in first chunk are <= median
    // and values in subsequent chunks are > median
    EXPECT_FALSE(chunks[0].empty());
    EXPECT_FALSE(chunks[1].empty());

    // Check first chunk (should contain values <= median)
    for (const double& value : chunks[0]) {
        EXPECT_LE(value, median);
    }

    // Check second chunk (should contain values > median)
    for (const double& value : chunks[1]) {
        EXPECT_GT(value, median);
    }
}

// Add more specific test cases
TEST_F(QuantileStrategyTest, EdgeCases) {
    // Empty input
    std::vector<double> empty_data;
    QuantileStrategy<double> strategy(0.5);
    auto empty_chunks = strategy.apply(empty_data);
    EXPECT_TRUE(empty_chunks.empty());

    // Single element
    std::vector<double> single_data = {1.0};
    auto single_chunks = strategy.apply(single_data);
    EXPECT_EQ(single_chunks.size(), 1);
    EXPECT_EQ(single_chunks[0].size(), 1);

    // All same values
    std::vector<double> same_data = {2.0, 2.0, 2.0, 2.0};
    auto same_chunks = strategy.apply(same_data);
    EXPECT_GT(same_chunks.size(), 0);
}

TEST_F(QuantileStrategyTest, InvalidQuantile) {
    EXPECT_THROW(QuantileStrategy<double>(-0.1), std::invalid_argument);
    EXPECT_THROW(QuantileStrategy<double>(1.1), std::invalid_argument);
    EXPECT_NO_THROW(QuantileStrategy<double>(0.0));
    EXPECT_NO_THROW(QuantileStrategy<double>(1.0));
    EXPECT_NO_THROW(QuantileStrategy<double>(0.5));
}

TEST_F(VarianceStrategyTest, BasicOperation) {
    VarianceStrategy<double> strategy(1.0);

    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 1);

    // Verify each chunk has low variance
    for (const auto& chunk : chunks) {
        double sum = std::accumulate(chunk.begin(), chunk.end(), 0.0);
        double mean = sum / chunk.size();
        double sq_sum = std::inner_product(chunk.begin(), chunk.end(), chunk.begin(), 0.0);
        double variance = sq_sum / chunk.size() - mean * mean;

        EXPECT_LE(variance, 1.0);
    }
}

TEST_F(EntropyStrategyTest, BasicOperation) {
    EntropyStrategy<double> strategy(1.5);

    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 1);

    // Verify chunks with repeated values have low entropy
    for (const auto& chunk : chunks) {
        std::map<int, int> freq;
        for (int val : chunk) {
            freq[val]++;
        }

        double entropy = 0.0;
        for (const auto& [_, count] : freq) {
            double p = static_cast<double>(count) / chunk.size();
            entropy -= p * std::log2(p);
        }

        EXPECT_LE(entropy, 1.5);
    }
}

TEST_F(QuantileStrategyTest, EmptyData) {
    std::vector<double> empty_data;
    auto strategy = QuantileStrategy<double>(0.5);
    EXPECT_TRUE(strategy.apply(empty_data).empty());
}

TEST_F(VarianceStrategyTest, SingleElement) {
    std::vector<double> single_data{1.0};
    auto strategy = VarianceStrategy<double>(1.0);
    auto chunks = strategy.apply(single_data);
    EXPECT_EQ(chunks.size(), 1);
}

TEST_F(EntropyStrategyTest, ConstantData) {
    std::vector<double> constant_data(10, 1.0);
    auto strategy = EntropyStrategy<double>(0.5);
    auto chunks = strategy.apply(constant_data);
    EXPECT_EQ(chunks.size(), 1);
}