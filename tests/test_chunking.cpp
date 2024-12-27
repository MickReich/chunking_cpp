#include "chunk_strategies.hpp"
#include <gtest/gtest.h>
#include <vector>

class ChunkingTest : public ::testing::Test {
protected:
    std::vector<int> test_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
};

TEST_F(ChunkingTest, BasicChunkingOperations) {
    // Test fixed-size chunking
    chunk_processing::PatternBasedStrategy<int> fixed_size_strategy(3);
    auto fixed_chunks = fixed_size_strategy.apply(test_data);

    EXPECT_EQ(fixed_chunks.size(), 4); // Should have 4 chunks (3,3,3,1)
    EXPECT_EQ(fixed_chunks[0].size(), 3);
    EXPECT_EQ(fixed_chunks[1].size(), 3);
    EXPECT_EQ(fixed_chunks[2].size(), 3);
    EXPECT_EQ(fixed_chunks[3].size(), 1);
}

TEST_F(ChunkingTest, PredicateBasedChunking) {
    // Test predicate-based chunking
    auto predicate = [](int val) { return val % 3 == 0; };
    chunk_processing::PatternBasedStrategy<int> pred_strategy(predicate);
    auto pred_chunks = pred_strategy.apply(test_data);

    EXPECT_GT(pred_chunks.size(), 0);
    for (const auto& chunk : pred_chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}

TEST_F(ChunkingTest, EmptyInput) {
    std::vector<int> empty_data;
    chunk_processing::PatternBasedStrategy<int> strategy(3);
    auto chunks = strategy.apply(empty_data);

    EXPECT_TRUE(chunks.empty());
}

TEST_F(ChunkingTest, SingleElementInput) {
    std::vector<int> single_element = {1};
    chunk_processing::PatternBasedStrategy<int> strategy(3);
    auto chunks = strategy.apply(single_element);

    EXPECT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].size(), 1);
}

TEST_F(ChunkingTest, ExactDivisionChunking) {
    std::vector<int> even_data = {1, 2, 3, 4, 5, 6};
    chunk_processing::PatternBasedStrategy<int> strategy(2);
    auto chunks = strategy.apply(even_data);

    EXPECT_EQ(chunks.size(), 3);
    for (const auto& chunk : chunks) {
        EXPECT_EQ(chunk.size(), 2);
    }
}