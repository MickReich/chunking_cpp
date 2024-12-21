#include "chunk_compression.hpp"
#include "gtest/gtest.h"

using namespace chunk_compression;

class ChunkCompressorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1, 1, 1, 2, 2, 3, 4, 4, 4, 4};
        empty_data = {};
        single_data = {42};
        same_data = std::vector<int>(5, 7);
    }

    std::vector<int> test_data;
    std::vector<int> empty_data;
    std::vector<int> single_data;
    std::vector<int> same_data;
};

TEST_F(ChunkCompressorTest, RunLengthEncoding) {
    auto encoded = ChunkCompressor<int>::run_length_encode(test_data);

    EXPECT_EQ(encoded.size(), 4);
    EXPECT_EQ(encoded[0], (std::pair<int, size_t>{1, 3}));
    EXPECT_EQ(encoded[1], (std::pair<int, size_t>{2, 2}));
    EXPECT_EQ(encoded[2], (std::pair<int, size_t>{3, 1}));
    EXPECT_EQ(encoded[3], (std::pair<int, size_t>{4, 4}));
}

TEST_F(ChunkCompressorTest, DeltaEncoding) {
    std::vector<int> data = {10, 12, 15, 19, 24};
    auto encoded = ChunkCompressor<int>::delta_encode(data);
    auto decoded = ChunkCompressor<int>::delta_decode(encoded);

    EXPECT_EQ(encoded, (std::vector<int>{10, 2, 3, 4, 5}));
    EXPECT_EQ(decoded, data);
}

TEST_F(ChunkCompressorTest, EmptyCompression) {
    EXPECT_TRUE(ChunkCompressor<int>::run_length_encode(empty_data).empty());
    EXPECT_TRUE(ChunkCompressor<int>::delta_encode(empty_data).empty());
}

TEST_F(ChunkCompressorTest, SingleElementCompression) {
    auto rle = ChunkCompressor<int>::run_length_encode(single_data);
    EXPECT_EQ(rle.size(), 1);
    
    auto delta = ChunkCompressor<int>::delta_encode(single_data);
    EXPECT_EQ(delta.size(), 1);
}

TEST_F(ChunkCompressorTest, AllSameElements) {
    auto rle = ChunkCompressor<int>::run_length_encode(same_data);
    EXPECT_EQ(rle.size(), 1);
}