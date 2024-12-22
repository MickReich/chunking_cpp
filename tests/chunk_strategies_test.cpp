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
        low_variance_data = {1.0, 1.1, 0.9, 1.05, 0.95};   // Low variance group
        high_variance_data = {1.0, 5.0, 10.0, 15.0, 20.0}; // High variance group
        mixed_variance_data = {
            1.0,  1.1,  0.9, // Group 1 (low variance)
            5.0,  5.1,  4.9, // Group 2 (low variance)
            10.0, 15.0, 20.0 // Group 3 (high variance)
        };
        test_data = {1.0, 1.1, 1.2, 5.0, 5.1, 5.2, 2.0, 2.1, 2.2};
    }

    const double variance_tolerance = 1e-10;
    std::vector<double> test_data;
    std::vector<double> low_variance_data;
    std::vector<double> high_variance_data;
    std::vector<double> mixed_variance_data;
};

class EntropyStrategyTest : public ::testing::Test {
protected:
    void SetUp() override {
        low_entropy_data = {1, 1, 1, 1, 1};  // Zero entropy
        high_entropy_data = {1, 2, 3, 4, 5}; // High entropy (all different)
        mixed_entropy_data = {
            1, 1, 1,   // Group 1 (low entropy)
            2, 2, 2,   // Group 2 (low entropy)
            1, 2, 3, 4 // Group 3 (high entropy)
        };
        test_data = {1, 1, 1, 2, 2, 3, 4, 4, 4, 4};
    }

    const double entropy_tolerance = 1e-10;
    std::vector<double> test_data;
    std::vector<double> low_entropy_data;
    std::vector<double> high_entropy_data;
    std::vector<double> mixed_entropy_data;
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

    // Check second chunk (should contain values >= median)
    for (const double& value : chunks[1]) {
        EXPECT_GE(value, median);
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

TEST_F(VarianceStrategyTest, LowVarianceData) {
    VarianceStrategy<double> strategy(0.1);
    auto chunks = strategy.apply(low_variance_data);
    EXPECT_EQ(chunks.size(), 1) << "Low variance data should not be split";
}

TEST_F(VarianceStrategyTest, HighVarianceData) {
    VarianceStrategy<double> strategy(10.0);
    auto chunks = strategy.apply(high_variance_data);
    EXPECT_GT(chunks.size(), 1) << "High variance data should be split";
}

TEST_F(VarianceStrategyTest, MixedVarianceData) {
    VarianceStrategy<double> strategy(1.0);
    auto chunks = strategy.apply(mixed_variance_data);

    EXPECT_GT(chunks.size(), 1) << "Mixed variance data should be split";

    // Check each chunk's variance
    for (const auto& chunk : chunks) {
        double mean = std::accumulate(chunk.begin(), chunk.end(), 0.0) / chunk.size();
        double variance = 0.0;
        for (const auto& val : chunk) {
            variance += (val - mean) * (val - mean);
        }
        variance /= chunk.size();

        EXPECT_LE(variance, 1.0 + variance_tolerance) << "Chunk variance exceeds threshold";
    }
}

TEST_F(EntropyStrategyTest, LowEntropyData) {
    EntropyStrategy<double> strategy(0.1);
    auto chunks = strategy.apply(low_entropy_data);
    EXPECT_EQ(chunks.size(), 1) << "Low entropy data should not be split";
}

TEST_F(EntropyStrategyTest, HighEntropyData) {
    EntropyStrategy<double> strategy(1.0);
    auto chunks = strategy.apply(high_entropy_data);
    EXPECT_GT(chunks.size(), 1) << "High entropy data should be split";
}

TEST_F(EntropyStrategyTest, MixedEntropyData) {
    EntropyStrategy<double> strategy(1.0);
    auto chunks = strategy.apply(mixed_entropy_data);

    EXPECT_GT(chunks.size(), 1) << "Mixed entropy data should be split";

    // Check each chunk's entropy
    for (const auto& chunk : chunks) {
        std::map<double, int> freq;
        for (const auto& val : chunk) {
            freq[val]++;
        }

        double entropy = 0.0;
        for (const auto& [_, count] : freq) {
            double p = static_cast<double>(count) / chunk.size();
            entropy -= p * std::log2(p);
        }

        EXPECT_LE(entropy, 1.0 + entropy_tolerance) << "Chunk entropy exceeds threshold";
    }
}

TEST_F(VarianceStrategyTest, ZeroThreshold) {
    VarianceStrategy<double> strategy(0.0);
    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 1) << "Zero variance threshold should split most non-constant data";
}

TEST_F(EntropyStrategyTest, ZeroThreshold) {
    EntropyStrategy<double> strategy(0.0);
    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 1) << "Zero entropy threshold should split most non-constant data";
}