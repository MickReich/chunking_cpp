#include <gtest/gtest.h>
#include "chunk_strategies.hpp"
#include "chunk_windows.hpp"
#include <vector>
#include <cmath>
#include <climits>

using namespace chunk_strategies;
using namespace chunk_windows;

class AdvancedChunkStrategiesTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Time series-like data
        time_series_data = {1.0, 1.1, 1.0, 4.0, 4.1, 4.2, 1.5, 1.6, 8.0, 8.1};
        
        // Cyclic pattern data
        cyclic_data = {0, 1, 2, 3, 2, 1, 0, -1, -2, -1, 0, 1, 2};
        
        // Data with clear segments
        segmented_data = {1, 1, 2, 5, 5, 5, 9, 9, 2, 2, 2, 7, 7};
    }

    std::vector<double> time_series_data;
    std::vector<int> cyclic_data;
    std::vector<int> segmented_data;
};

// Pattern-Based Strategy Tests
TEST_F(AdvancedChunkStrategiesTest, PatternBasedLocalMaxima) {
    auto local_maxima_detector = [](double x) {
        static double prev = x;
        static double prev_prev = x;
        bool is_local_max = (prev > prev_prev && prev > x);
        prev_prev = prev;
        prev = x;
        return is_local_max;
    };

    PatternBasedStrategy<double> strategy(local_maxima_detector);
    auto chunks = strategy.apply(time_series_data);
    
    ASSERT_GT(chunks.size(), 1);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}

TEST_F(AdvancedChunkStrategiesTest, PatternBasedCyclicDetection) {
    auto cycle_detector = [](int x) {
        static std::vector<int> pattern;
        static int last_direction = 0;
        
        if (pattern.size() < 2) {
            pattern.push_back(x);
            return false;
        }
        
        int current_direction = x - pattern.back();
        bool start_new = (last_direction * current_direction < 0);  // Direction change when product is negative
        
        pattern.push_back(x);
        last_direction = current_direction;
        return start_new;
    };

    PatternBasedStrategy<int> strategy(cycle_detector);
    auto chunks = strategy.apply(cyclic_data);
    
    ASSERT_GT(chunks.size(), 2);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 1);  // Each chunk should contain at least 2 elements
    }
}
// Multi-Criteria Strategy Tests
TEST_F(AdvancedChunkStrategiesTest, MultiCriteriaWithSizeAndSimilarity) {
    MultiCriteriaStrategy<double> strategy(0.3, 3);
    auto chunks = strategy.apply(time_series_data);
    
    for (const auto& chunk : chunks) {
        // Check size constraint
        EXPECT_LE(chunk.size(), 3);
        
        // Check similarity constraint within chunk
        if (chunk.size() > 1) {
            for (size_t i = 1; i < chunk.size(); ++i) {
                EXPECT_LE(std::abs(chunk[i] - chunk[i-1]), 0.3);
            }
        }
    }
}

TEST_F(AdvancedChunkStrategiesTest, MultiCriteriaWithPatternAndSize) {
    MultiCriteriaStrategy<int> strategy(2, 4);  // max difference of 2, max size of 4
    auto chunks = strategy.apply(segmented_data);
    
    ASSERT_GT(chunks.size(), 1);
    for (const auto& chunk : chunks) {
        EXPECT_LE(chunk.size(), 4);
        if (chunk.size() > 1) {
            int max_diff = 0;
            for (size_t i = 1; i < chunk.size(); ++i) {
                max_diff = std::max(max_diff, std::abs(chunk[i] - chunk[i-1]));
            }
            EXPECT_LE(max_diff, 2);
        }
    }
}

// Dynamic Threshold Strategy Tests
TEST_F(AdvancedChunkStrategiesTest, DynamicThresholdWithDecay) {
    DynamicThresholdStrategy<double> strategy(1.0, 0.1, 0.9);
    auto chunks = strategy.apply(time_series_data);
    
    ASSERT_GT(chunks.size(), 1);
    // Verify that later chunks allow smaller variations
    std::vector<double> chunk_variations;
    for (const auto& chunk : chunks) {
        if (chunk.size() > 1) {
            double max_var = 0.0;
            for (size_t i = 1; i < chunk.size(); ++i) {
                max_var = std::max(max_var, std::abs(chunk[i] - chunk[i-1]));
            }
            chunk_variations.push_back(max_var);
        }
    }
    
    // Verify general trend of decreasing variations
    for (size_t i = 1; i < chunk_variations.size(); ++i) {
        EXPECT_LE(chunk_variations[i], chunk_variations[i-1] * 1.1);  // Allow 10% tolerance
    }
}

TEST_F(AdvancedChunkStrategiesTest, DynamicThresholdWithAdaptiveMinimum) {
    DynamicThresholdStrategy<int> strategy(5, 1, 0.8);
    auto chunks = strategy.apply(segmented_data);
    
    ASSERT_GT(chunks.size(), 1);
    int prev_max_diff = INT_MAX;
    for (const auto& chunk : chunks) {
        if (chunk.size() > 1) {
            int max_diff = 0;
            for (size_t i = 1; i < chunk.size(); ++i) {
                max_diff = std::max(max_diff, std::abs(chunk[i] - chunk[i-1]));
            }
            if (max_diff > 0) {
                EXPECT_LE(max_diff, prev_max_diff);
                EXPECT_GE(max_diff, 1);  // Lower minimum threshold check to 1
                prev_max_diff = max_diff;
            }
        }
    }
}

// Window Processing Tests
TEST_F(AdvancedChunkStrategiesTest, SlidingWindowWithCustomAggregation) {
    SlidingWindowProcessor<double> processor(3, 1);
    
    // Custom aggregation: range (max - min) in window
    auto range_calculator = [](const std::vector<double>& window) {
        if (window.empty()) return 0.0;
        auto [min_it, max_it] = std::minmax_element(window.begin(), window.end());
        return *max_it - *min_it;
    };
    
    auto results = processor.process(time_series_data, range_calculator);
    
    ASSERT_GT(results.size(), 1);
    for (double range : results) {
        EXPECT_GE(range, 0.0);
    }
}

TEST_F(AdvancedChunkStrategiesTest, WindowOperationsWithStatistics) {
    std::vector<double> window = {1.0, 2.0, 3.0, 4.0, 5.0};
    
    EXPECT_DOUBLE_EQ(WindowOperations<double>::moving_average(window), 3.0);
    EXPECT_DOUBLE_EQ(WindowOperations<double>::moving_median(window), 3.0);
    EXPECT_DOUBLE_EQ(WindowOperations<double>::moving_max(window), 5.0);
    EXPECT_DOUBLE_EQ(WindowOperations<double>::moving_min(window), 1.0);
}

// Edge Cases and Boundary Tests
TEST_F(AdvancedChunkStrategiesTest, SingleElementInput) {
    std::vector<double> single_element = {1.0};
    
    PatternBasedStrategy<double> pattern_strategy([](double) { return true; });
    EXPECT_EQ(pattern_strategy.apply(single_element).size(), 1);
    
    AdaptiveStrategy<double> adaptive_strategy(1.0, [](const std::vector<double>&) { return 0.0; });
    EXPECT_EQ(adaptive_strategy.apply(single_element).size(), 1);
    
    MultiCriteriaStrategy<double> multi_strategy(0.5, 2);
    EXPECT_EQ(multi_strategy.apply(single_element).size(), 1);
    
    DynamicThresholdStrategy<double> dynamic_strategy(1.0, 0.1, 0.9);
    EXPECT_EQ(dynamic_strategy.apply(single_element).size(), 1);
}

TEST_F(AdvancedChunkStrategiesTest, LargeValueRanges) {
    std::vector<double> large_range = {1e-6, 1e-3, 1.0, 1e3, 1e6};
    
    DynamicThresholdStrategy<double> strategy(1e3, 1.0, 0.9);
    auto chunks = strategy.apply(large_range);
    
    ASSERT_GT(chunks.size(), 1);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
} 