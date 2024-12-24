/**
 * @file chunk_test.cpp
 * @brief Unit tests for the Chunk class.
 *
 * This file contains tests for various operations on the Chunk class,
 * including adding elements, retrieving chunks, and handling edge cases.
 */

#include "chunk.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class ChunkTest : public ::testing::Test {
protected:
    using value_type = int;
    chunk_processing::Chunk<value_type> basic_chunker{2};
    std::vector<value_type> test_data{1, 2, 3, 4, 5};
};

TEST_F(ChunkTest, Constructor) {
    EXPECT_THROW(chunk_processing::Chunk<value_type>(0), std::invalid_argument);
    EXPECT_NO_THROW(chunk_processing::Chunk<value_type>(1));
}

TEST_F(ChunkTest, SingleElementAdd) {
    basic_chunker.add(1);
    EXPECT_EQ(basic_chunker.size(), 1);
}

TEST_F(ChunkTest, VectorAdd) {
    basic_chunker.add(test_data);
    EXPECT_EQ(basic_chunker.size(), test_data.size());
}

TEST_F(ChunkTest, ChunkBySize) {
    basic_chunker.add(test_data);
    auto chunks = basic_chunker.chunk_by_size(2);
    EXPECT_EQ(chunks.size(), 3); // Should create 3 chunks: [1,2], [3,4], [5]
}

TEST_F(ChunkTest, ThresholdChunking) {
    basic_chunker.add({1, 2, 5, 6, 1, 2, 7, 8});
    auto chunks = basic_chunker.chunk_by_threshold(3);
    EXPECT_GT(chunks.size(), 1);
}

TEST_F(ChunkTest, BasicOperations) {
    chunk_processing::Chunk<int> chunk(2);
    ASSERT_EQ(chunk.size(), 0);              // Initially empty
    ASSERT_EQ(chunk.get_chunks().size(), 0); // No chunks yet
}

TEST_F(ChunkTest, ChunkProcessing) {
    chunk_processing::Chunk<int> chunk(2); // Create chunk with size 2
    chunk.add(test_data);                  // Add data from fixture
    auto chunks = chunk.get_chunks();
    EXPECT_FALSE(chunks.empty());
}

TEST_F(ChunkTest, EmptyChunkOperations) {
    EXPECT_EQ(basic_chunker.size(), 0);
    EXPECT_TRUE(basic_chunker.get_chunks().empty());
}

TEST_F(ChunkTest, EdgeCases) {
    std::vector<value_type> empty_data;
    basic_chunker.add(empty_data);
    EXPECT_EQ(basic_chunker.size(), 0);
}

TEST_F(ChunkTest, ExactSizeChunking) {
    chunk_processing::Chunk<value_type> exact_chunker(5);
    exact_chunker.add(test_data);
    auto chunks = exact_chunker.get_chunks();
    EXPECT_EQ(chunks.size(), 1);
}

TEST_F(ChunkTest, StringChunking) {
    chunk_processing::Chunk<std::string> string_chunker(2);
    std::vector<std::string> str_data{"hello", "world", "test"};
    string_chunker.add(str_data);
    auto chunks = string_chunker.get_chunks();
    EXPECT_FALSE(chunks.empty());
}

TEST_F(ChunkTest, CharChunking) {
    chunk_processing::Chunk<char> char_chunker(3);
    std::vector<char> char_data{'a', 'b', 'c', 'd', 'e'};
    char_chunker.add(char_data);
    auto chunks = char_chunker.get_chunks();
    EXPECT_FALSE(chunks.empty());
}