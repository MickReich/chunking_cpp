#include "../include/chunk.hpp"
#include <gtest/gtest.h>

class ChunkTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1, 2, 3, 4, 5};
    }

    std::vector<int> test_data;
    Chunk<int> basic_chunker{2};
};

TEST_F(ChunkTest, Constructor) {
    EXPECT_THROW(Chunk<int>(0), std::invalid_argument);
    EXPECT_NO_THROW(Chunk<int>(1));
}

TEST_F(ChunkTest, AddSingleElement) {
    basic_chunker.add(1);
    EXPECT_EQ(basic_chunker.size(), 1);
    auto chunks = basic_chunker.get_chunks();
    EXPECT_EQ(chunks.size(), 1);
    EXPECT_EQ(chunks[0].size(), 1);
    EXPECT_EQ(chunks[0][0], 1);
}

TEST_F(ChunkTest, AddVector) {
    basic_chunker.add(test_data);
    EXPECT_EQ(basic_chunker.size(), 5);
    auto chunks = basic_chunker.get_chunks();
    EXPECT_EQ(chunks.size(), 3); // [1,2], [3,4], [5]
}

TEST_F(ChunkTest, GetChunk) {
    basic_chunker.add(test_data);
    auto chunk = basic_chunker.get_chunk(0);
    EXPECT_EQ(chunk.size(), 2);
    EXPECT_EQ(chunk[0], 1);
    EXPECT_EQ(chunk[1], 2);
    EXPECT_THROW(basic_chunker.get_chunk(3), std::out_of_range);
}

TEST_F(ChunkTest, OverlappingChunks) {
    basic_chunker.add(test_data);
    EXPECT_THROW(basic_chunker.get_overlapping_chunks(2), std::invalid_argument);

    auto chunks = basic_chunker.get_overlapping_chunks(1);
    EXPECT_EQ(chunks.size(), 4);
}

TEST_F(ChunkTest, PredicateChunking) {
    basic_chunker.add(test_data);
    auto chunks = basic_chunker.chunk_by_predicate([](int x) { return x % 2 == 0; });
    EXPECT_EQ(chunks.size(), 3); // [1], [2,3], [4,5]
}

TEST_F(ChunkTest, SumChunking) {
    basic_chunker.add(test_data);
    auto chunks = basic_chunker.chunk_by_sum(3);
    EXPECT_EQ(chunks.size(), 4);

    ASSERT_GE(chunks.size(), 1);
    EXPECT_EQ(chunks[0], (std::vector<int>{1, 2}));
    EXPECT_EQ(chunks[1], (std::vector<int>{3}));
    EXPECT_EQ(chunks[2], (std::vector<int>{4}));
    EXPECT_EQ(chunks[3], (std::vector<int>{5}));
}

TEST_F(ChunkTest, EqualDivision) {
    basic_chunker.add(test_data);
    auto chunks = basic_chunker.chunk_into_n(2);
    EXPECT_EQ(chunks.size(), 2);
    EXPECT_EQ(chunks[0].size(), 3); // [1,2,3]
    EXPECT_EQ(chunks[1].size(), 2); // [4,5]
}