#include "parallel_chunk.hpp"
#include "gtest/gtest.h"
#include <numeric>

using namespace parallel_chunk;

TEST(ParallelChunkProcessorTest, BasicProcessing) {
    std::vector<std::vector<int>> chunks = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    // Double each number in parallel
    auto operation = [](std::vector<int>& chunk) {
        for (int& val : chunk) {
            val *= 2;
        }
    };

    ParallelChunkProcessor<int>::process_chunks(chunks, operation);

    EXPECT_EQ(chunks[0], (std::vector<int>{2, 4, 6}));
    EXPECT_EQ(chunks[1], (std::vector<int>{8, 10, 12}));
    EXPECT_EQ(chunks[2], (std::vector<int>{14, 16, 18}));
}

TEST(ParallelChunkProcessorTest, MapReduce) {
    std::vector<std::vector<int>> chunks = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    // Map: Square each number
    auto square = [](const int& x) { return x * x; };
    auto squared_chunks = ParallelChunkProcessor<int>::map<int>(chunks, square);

    // Reduce: Sum all squares
    auto sum = [](const int& a, const int& b) { return a + b; };
    int result = ParallelChunkProcessor<int>::reduce(squared_chunks, sum, 0);

    // Expected: 1^2 + 2^2 + ... + 9^2 = 285
    EXPECT_EQ(result, 285);
}