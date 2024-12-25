#include "chunk_strategies.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace chunk_processing;

class QuantileStrategyTest : public ::testing::Test {
protected:
    std::vector<double> test_data = {1.0, 2.0, 3.0, 10.0, 11.0, 12.0};
    std::vector<double> empty_data = {};
};

class VarianceStrategyTest : public ::testing::Test {
protected:
    std::vector<double> test_data = {1.0, 1.1, 1.2, 5.0, 5.1, 5.2};
    std::vector<double> low_variance_data = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5};
    std::vector<double> high_variance_data = {1.0, 10.0, 20.0, 30.0, 40.0, 50.0};
    std::vector<double> mixed_variance_data = {1.0, 1.1, 10.0, 10.1, 20.0, 20.1};
};

class EntropyStrategyTest : public ::testing::Test {
protected:
    std::vector<double> test_data = {1.0, 2.0, 3.0, 1.0, 2.0, 3.0};
    std::vector<double> low_entropy_data = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    std::vector<double> high_entropy_data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    std::vector<double> mixed_entropy_data = {1.0, 1.0, 2.0, 2.0, 3.0, 3.0};
};

TEST_F(VarianceStrategyTest, BasicOperation) {
    VarianceStrategy<double> strategy(1.0);
    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(EntropyStrategyTest, BasicOperation) {
    EntropyStrategy<double> strategy(1.5);
    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 0);
}

TEST_F(VarianceStrategyTest, SingleElement) {
    auto strategy = VarianceStrategy<double>(1.0);
    std::vector<double> single_element = {1.0};
    auto chunks = strategy.apply(single_element);
    EXPECT_EQ(chunks.size(), 1);
}

TEST_F(EntropyStrategyTest, ConstantData) {
    auto strategy = EntropyStrategy<double>(0.5);
    auto chunks = strategy.apply(low_entropy_data);
    EXPECT_EQ(chunks.size(), 1);
}

TEST_F(VarianceStrategyTest, LowVarianceData) {
    VarianceStrategy<double> strategy(0.1);
    auto chunks = strategy.apply(low_variance_data);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(VarianceStrategyTest, HighVarianceData) {
    VarianceStrategy<double> strategy(10.0);
    auto chunks = strategy.apply(high_variance_data);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(VarianceStrategyTest, MixedVarianceData) {
    VarianceStrategy<double> strategy(1.0);
    auto chunks = strategy.apply(mixed_variance_data);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(EntropyStrategyTest, LowEntropyData) {
    EntropyStrategy<double> strategy(0.1);
    auto chunks = strategy.apply(low_entropy_data);
    EXPECT_EQ(chunks.size(), 1);
}

TEST_F(EntropyStrategyTest, HighEntropyData) {
    EntropyStrategy<double> strategy(1.0);
    auto chunks = strategy.apply(high_entropy_data);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(EntropyStrategyTest, MixedEntropyData) {
    EntropyStrategy<double> strategy(1.0);
    auto chunks = strategy.apply(mixed_entropy_data);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(VarianceStrategyTest, ZeroThreshold) {
    VarianceStrategy<double> strategy(0.0);
    auto chunks = strategy.apply(test_data);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(EntropyStrategyTest, ZeroThreshold) {
    EntropyStrategy<double> strategy(0.0);
    auto chunks = strategy.apply(test_data);
    EXPECT_EQ(chunks.size(), 1);
}