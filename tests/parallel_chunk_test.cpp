#include "parallel_chunk.hpp"
#include "gtest/gtest.h"
#include <numeric>

using namespace parallel_chunk;

class ParallelChunkProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        chunks = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    }

    std::vector<int> test_data;
    std::vector<std::vector<int>> chunks;
};

TEST_F(ParallelChunkProcessorTest, BasicProcessing) {
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

TEST_F(ParallelChunkProcessorTest, MapReduce) {
    // Map: Square each number
    auto square = [](const int& x) { return x * x; };
    auto squared_chunks = ParallelChunkProcessor<int>::map<int>(chunks, square);

    // Reduce: Sum all squares
    auto sum = [](const int& a, const int& b) { return a + b; };
    int result = ParallelChunkProcessor<int>::reduce(squared_chunks, sum, 0);

    // Expected: 1^2 + 2^2 + ... + 9^2 = 285
    EXPECT_EQ(result, 285);
}

TEST_F(ParallelChunkProcessorTest, EmptyProcessing) {
    std::vector<std::vector<int>> empty_data;
    ParallelChunkProcessor<int>::process_chunks(empty_data, [](std::vector<int>& chunk) {
        for (int& x : chunk) {
            x *= 2;
        }
    });
    EXPECT_TRUE(empty_data.empty());
}

TEST_F(ParallelChunkProcessorTest, SingleThreadProcessing) {
    std::vector<std::vector<int>> chunked_data = {test_data};
    ParallelChunkProcessor<int>::process_chunks(chunked_data, [](std::vector<int>& chunk) {
        for (int& x : chunk) {
            x *= 2;
        }
    });
    EXPECT_EQ(chunked_data[0].size(), test_data.size());
    for (size_t i = 0; i < test_data.size(); ++i) {
        EXPECT_EQ(chunked_data[0][i], test_data[i] * 2);
    }
}

TEST_F(ParallelChunkProcessorTest, ExceptionHandling) {
    std::vector<std::vector<int>> chunked_data = {test_data};
    EXPECT_THROW(
        ParallelChunkProcessor<int>::process_chunks(chunked_data, [](std::vector<int>& chunk) {
            for (size_t i = 0; i < chunk.size(); ++i) {
                int x = chunk[i];
                if (x > 3) throw std::runtime_error("test");
                chunk[i] = x * 2;
            }
        }),
        std::runtime_error
    );
}