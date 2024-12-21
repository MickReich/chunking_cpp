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

TEST_F(ChunkCompressorTest, RunLengthEdgeCases) {
    // Test alternating values
    std::vector<int> alternating = {1, 2, 1, 2, 1, 2};
    auto encoded = ChunkCompressor<int>::run_length_encode(alternating);
    EXPECT_EQ(encoded.size(), 6);
    for (size_t i = 0; i < encoded.size(); ++i) {
        EXPECT_EQ(encoded[i].first, (i % 2) + 1);
        EXPECT_EQ(encoded[i].second, 1);
    }

    // Test single repeated value
    std::vector<int> repeated(5, 42);
    auto encoded_repeated = ChunkCompressor<int>::run_length_encode(repeated);
    EXPECT_EQ(encoded_repeated.size(), 1);
    EXPECT_EQ(encoded_repeated[0], (std::pair<int, size_t>{42, 5}));
}

TEST_F(ChunkCompressorTest, DeltaEdgeCases) {
    // Test constant sequence
    std::vector<int> constant(5, 7);
    auto encoded_constant = ChunkCompressor<int>::delta_encode(constant);
    EXPECT_EQ(encoded_constant[0], 7);
    for (size_t i = 1; i < encoded_constant.size(); ++i) {
        EXPECT_EQ(encoded_constant[i], 0);
    }

    // Test decreasing sequence
    std::vector<int> decreasing = {10, 8, 6, 4, 2};
    auto encoded_decreasing = ChunkCompressor<int>::delta_encode(decreasing);
    EXPECT_EQ(encoded_decreasing[0], 10);
    for (size_t i = 1; i < encoded_decreasing.size(); ++i) {
        EXPECT_EQ(encoded_decreasing[i], -2);
    }
}

TEST_F(ChunkCompressorTest, DeltaRoundTrip) {
    // Test various sequences
    std::vector<std::vector<int>> test_sequences = {
        {1, 1, 1, 1},      // Constant
        {1, 2, 3, 4, 5},   // Linear increasing
        {5, 4, 3, 2, 1},   // Linear decreasing
        {1, 3, 6, 10, 15}, // Quadratic
        {-2, -1, 0, 1, 2}  // Negative to positive
    };

    for (const auto& sequence : test_sequences) {
        auto encoded = ChunkCompressor<int>::delta_encode(sequence);
        auto decoded = ChunkCompressor<int>::delta_decode(encoded);
        EXPECT_EQ(decoded, sequence) << "Failed for sequence starting with " << sequence[0];
    }
}

TEST_F(ChunkCompressorTest, LargeNumbers) {
    std::vector<int> large_nums = {1000000, 1000001, 1000002, 1000003};

    // Test RLE with large numbers
    auto rle = ChunkCompressor<int>::run_length_encode(large_nums);
    EXPECT_EQ(rle.size(), 4);
    for (size_t i = 0; i < rle.size(); ++i) {
        EXPECT_EQ(rle[i].first, 1000000 + i);
        EXPECT_EQ(rle[i].second, 1);
    }

    // Test delta encoding with large numbers
    auto delta = ChunkCompressor<int>::delta_encode(large_nums);
    EXPECT_EQ(delta[0], 1000000);
    for (size_t i = 1; i < delta.size(); ++i) {
        EXPECT_EQ(delta[i], 1);
    }
}

TEST_F(ChunkCompressorTest, MixedPatterns) {
    std::vector<int> mixed = {1, 1, 1, 2, 3, 3, 4, 4, 4, 4};

    // Test RLE with mixed patterns
    auto rle = ChunkCompressor<int>::run_length_encode(mixed);
    EXPECT_EQ(rle.size(), 4);
    EXPECT_EQ(rle[0], (std::pair<int, size_t>{1, 3}));
    EXPECT_EQ(rle[1], (std::pair<int, size_t>{2, 1}));
    EXPECT_EQ(rle[2], (std::pair<int, size_t>{3, 2}));
    EXPECT_EQ(rle[3], (std::pair<int, size_t>{4, 4}));

    // Test delta encoding with mixed patterns
    auto delta = ChunkCompressor<int>::delta_encode(mixed);
    auto decoded = ChunkCompressor<int>::delta_decode(delta);
    EXPECT_EQ(decoded, mixed);
}

TEST_F(ChunkCompressorTest, CompressionRatio) {
    // Create a highly compressible sequence
    std::vector<int> compressible(100, 42);
    auto rle = ChunkCompressor<int>::run_length_encode(compressible);
    EXPECT_EQ(rle.size(), 1);
    EXPECT_EQ(rle[0].second, 100);

    // Create a sequence with poor compression
    std::vector<int> incompressible;
    for (int i = 0; i < 100; ++i) {
        incompressible.push_back(i);
    }
    auto rle_incomp = ChunkCompressor<int>::run_length_encode(incompressible);
    EXPECT_EQ(rle_incomp.size(), 100);
}