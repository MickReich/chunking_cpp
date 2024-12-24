#include "chunk_strategies.hpp"
#include "chunk_windows.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <numeric>

class AdvancedChunkStrategiesTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1.0, 2.0, 5.0, 6.0, 1.0, 2.0, 7.0, 8.0};
        cyclic_data = {1, 2, 3, 1, 2, 3, 1, 2, 3};
    }

    std::vector<double> test_data;
    std::vector<int> cyclic_data;
};

TEST_F(AdvancedChunkStrategiesTest, PatternBasedLocalMaxima) {
    // Reset static variables for each test
    auto local_maxima_detector = [](double x) {
        thread_local double prev = x;
        thread_local double prev_prev = x;
        bool is_local_max = (prev > prev_prev && prev > x);
        prev_prev = prev;
        prev = x;
        return is_local_max;
    };

    chunk_strategies::PatternBasedStrategy<double> strategy(local_maxima_detector);
    auto chunks = strategy.apply(test_data);
    
    EXPECT_GT(chunks.size(), 1);
    for (const auto& chunk : chunks) {
        EXPECT_FALSE(chunk.empty());
    }
}

TEST_F(AdvancedChunkStrategiesTest, PatternBasedCyclicDetection) {
    // Test size-based constructor
    chunk_strategies::PatternBasedStrategy<int> strategy(3); // Pattern size of 3
    auto chunks = strategy.apply(cyclic_data);
    
    EXPECT_EQ(chunks.size(), 3); // Should detect 3 complete cycles
    for (const auto& chunk : chunks) {
        EXPECT_EQ(chunk.size(), 3);
    }
}

TEST_F(AdvancedChunkStrategiesTest, SingleElementInput) {
    std::vector<double> single_element{1.0};
    
    // Test both constructors with single element
    chunk_strategies::PatternBasedStrategy<double> size_strategy(2);
    auto size_chunks = size_strategy.apply(single_element);
    EXPECT_EQ(size_chunks.size(), 1);
    
    chunk_strategies::PatternBasedStrategy<double> pred_strategy([](double) { return true; });
    auto pred_chunks = pred_strategy.apply(single_element);
    EXPECT_EQ(pred_chunks.size(), 1);
}

TEST_F(AdvancedChunkStrategiesTest, EmptyInput) {
    std::vector<double> empty_data;
    
    // Test both constructors with empty input
    chunk_strategies::PatternBasedStrategy<double> size_strategy(2);
    auto size_chunks = size_strategy.apply(empty_data);
    EXPECT_TRUE(size_chunks.empty());
    
    chunk_strategies::PatternBasedStrategy<double> pred_strategy([](double) { return true; });
    auto pred_chunks = pred_strategy.apply(empty_data);
    EXPECT_TRUE(pred_chunks.empty());
}

// Add tests for window processing
TEST_F(AdvancedChunkStrategiesTest, WindowProcessing) {
    chunk_windows::SlidingWindowProcessor<double> processor(3, 1);
    auto result = processor.process(test_data, [](const std::vector<double>& window) {
        return std::accumulate(window.begin(), window.end(), 0.0) / window.size();
    });
    
    EXPECT_FALSE(result.empty());
    EXPECT_LE(result.size(), test_data.size());
}

// Test window operations
TEST_F(AdvancedChunkStrategiesTest, WindowOperations) {
    std::vector<double> window{1.0, 2.0, 3.0, 4.0, 5.0};
    
    EXPECT_EQ(chunk_windows::WindowOperations<double>::moving_average(window), 3.0);
    EXPECT_EQ(chunk_windows::WindowOperations<double>::moving_median(window), 3.0);
    EXPECT_EQ(chunk_windows::WindowOperations<double>::moving_max(window), 5.0);
    EXPECT_EQ(chunk_windows::WindowOperations<double>::moving_min(window), 1.0);
}

TEST_F(AdvancedChunkStrategiesTest, WindowOperationsEmptyInput) {
    std::vector<double> empty_window;
    EXPECT_EQ(chunk_windows::WindowOperations<double>::moving_average(empty_window), 0.0);
    EXPECT_THROW(chunk_windows::WindowOperations<double>::moving_median(empty_window), 
                 std::invalid_argument);
    EXPECT_THROW(chunk_windows::WindowOperations<double>::moving_max(empty_window), 
                 std::invalid_argument);
    EXPECT_THROW(chunk_windows::WindowOperations<double>::moving_min(empty_window), 
                 std::invalid_argument);
}