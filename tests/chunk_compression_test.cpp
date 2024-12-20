#include "chunk_compression.hpp"
#include "gtest/gtest.h"

using namespace chunk_compression;

TEST(ChunkCompressorTest, RunLengthEncoding) {
    std::vector<int> data = {1, 1, 1, 2, 2, 3, 4, 4, 4, 4};
    auto encoded = ChunkCompressor<int>::run_length_encode(data);

    EXPECT_EQ(encoded.size(), 4);
    EXPECT_EQ(encoded[0], (std::pair<int, size_t>{1, 3}));
    EXPECT_EQ(encoded[1], (std::pair<int, size_t>{2, 2}));
    EXPECT_EQ(encoded[2], (std::pair<int, size_t>{3, 1}));
    EXPECT_EQ(encoded[3], (std::pair<int, size_t>{4, 4}));
}

TEST(ChunkCompressorTest, DeltaEncoding) {
    std::vector<int> data = {10, 12, 15, 19, 24};
    auto encoded = ChunkCompressor<int>::delta_encode(data);
    auto decoded = ChunkCompressor<int>::delta_decode(encoded);

    EXPECT_EQ(encoded, (std::vector<int>{10, 2, 3, 4, 5}));
    EXPECT_EQ(decoded, data);
}