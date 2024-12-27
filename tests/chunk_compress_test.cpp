#include "chunk_compression.hpp"
#include <cstdint> // for uint64_t
#include <gtest/gtest.h>
#include <limits>
#include <numeric> // for std::iota
#include <utility> // for std::pair
#include <vector>

using namespace chunk_compression;

template <typename T>
bool ComparePairs(const std::pair<T, size_t>& lhs, const std::pair<T, size_t>& rhs) {
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

class ChunkCompressorTest : public ::testing::Test {
protected:
    std::vector<int> repeated_data = {1, 1, 1, 2, 2, 3, 3, 3, 3};
    std::vector<int> single_value = {5};
    std::vector<int> unique_data = {1, 2, 3, 4, 5};
    std::vector<int> empty_data = {};
    std::vector<double> floating_data = {1.5, 1.5, 2.5, 2.5, 2.5};
};

// Run-Length Encoding Tests
TEST_F(ChunkCompressorTest, BasicRunLengthEncode) {
    auto encoded = ChunkCompressor<int>::run_length_encode(repeated_data);
    EXPECT_EQ(encoded.size(), 3);
    EXPECT_TRUE(ComparePairs(encoded[0], std::make_pair(1, size_t(3))));
    EXPECT_TRUE(ComparePairs(encoded[1], std::make_pair(2, size_t(2))));
    EXPECT_TRUE(ComparePairs(encoded[2], std::make_pair(3, size_t(4))));
}

TEST_F(ChunkCompressorTest, SingleValueRunLength) {
    auto encoded = ChunkCompressor<int>::run_length_encode(single_value);
    EXPECT_EQ(encoded.size(), 1);
    EXPECT_TRUE(ComparePairs(encoded[0], std::make_pair(5, size_t(1))));
}

TEST_F(ChunkCompressorTest, UniqueValuesRunLength) {
    auto encoded = ChunkCompressor<int>::run_length_encode(unique_data);
    EXPECT_EQ(encoded.size(), unique_data.size());
    for (size_t i = 0; i < encoded.size(); ++i) {
        EXPECT_TRUE(ComparePairs(encoded[i], std::make_pair(unique_data[i], size_t(1))));
    }
}

TEST_F(ChunkCompressorTest, EmptyRunLength) {
    auto encoded = ChunkCompressor<int>::run_length_encode(empty_data);
    EXPECT_TRUE(encoded.empty());
}

TEST_F(ChunkCompressorTest, FloatingPointRunLength) {
    auto encoded = ChunkCompressor<double>::run_length_encode(floating_data);
    EXPECT_EQ(encoded.size(), 2);
    EXPECT_TRUE(ComparePairs(encoded[0], std::make_pair(1.5, size_t(2))));
    EXPECT_TRUE(ComparePairs(encoded[1], std::make_pair(2.5, size_t(3))));
}

// Delta Encoding Tests
TEST_F(ChunkCompressorTest, BasicDeltaEncode) {
    std::vector<int> sequence = {10, 13, 15, 16, 20};
    auto encoded = ChunkCompressor<int>::delta_encode(sequence);
    EXPECT_EQ(encoded.size(), sequence.size());
    EXPECT_EQ(encoded[0], 10); // First value unchanged
    EXPECT_EQ(encoded[1], 3);  // 13 - 10
    EXPECT_EQ(encoded[2], 2);  // 15 - 13
    EXPECT_EQ(encoded[3], 1);  // 16 - 15
    EXPECT_EQ(encoded[4], 4);  // 20 - 16
}

TEST_F(ChunkCompressorTest, DeltaEncodeConstantSequence) {
    std::vector<int> constant = {5, 5, 5, 5, 5};
    auto encoded = ChunkCompressor<int>::delta_encode(constant);
    EXPECT_EQ(encoded[0], 5);
    for (size_t i = 1; i < encoded.size(); ++i) {
        EXPECT_EQ(encoded[i], 0);
    }
}

TEST_F(ChunkCompressorTest, DeltaEncodeEmpty) {
    auto encoded = ChunkCompressor<int>::delta_encode(empty_data);
    EXPECT_TRUE(encoded.empty());
}

TEST_F(ChunkCompressorTest, DeltaEncodeSingle) {
    auto encoded = ChunkCompressor<int>::delta_encode(single_value);
    EXPECT_EQ(encoded.size(), 1);
    EXPECT_EQ(encoded[0], single_value[0]);
}

// Delta Decoding Tests
TEST_F(ChunkCompressorTest, DeltaEncodeDecode) {
    std::vector<int> original = {10, 13, 15, 16, 20};
    auto encoded = ChunkCompressor<int>::delta_encode(original);
    auto decoded = ChunkCompressor<int>::delta_decode(encoded);
    EXPECT_EQ(decoded, original);
}

TEST_F(ChunkCompressorTest, DeltaDecodeEmpty) {
    auto decoded = ChunkCompressor<int>::delta_decode(empty_data);
    EXPECT_TRUE(decoded.empty());
}

TEST_F(ChunkCompressorTest, DeltaDecodeSingle) {
    auto decoded = ChunkCompressor<int>::delta_decode(single_value);
    EXPECT_EQ(decoded, single_value);
}

// Edge Cases and Special Values
TEST_F(ChunkCompressorTest, LargeDeltas) {
    std::vector<int> large_values = {std::numeric_limits<int>::max() - 2,
                                     std::numeric_limits<int>::max() - 1,
                                     std::numeric_limits<int>::max()};
    auto encoded = ChunkCompressor<int>::delta_encode(large_values);
    auto decoded = ChunkCompressor<int>::delta_decode(encoded);
    EXPECT_EQ(decoded, large_values);
}

TEST_F(ChunkCompressorTest, NegativeDeltas) {
    std::vector<int> alternating = {5, 3, 8, 1, 6};
    auto encoded = ChunkCompressor<int>::delta_encode(alternating);
    auto decoded = ChunkCompressor<int>::delta_decode(encoded);
    EXPECT_EQ(decoded, alternating);
}

TEST_F(ChunkCompressorTest, FloatingPointDelta) {
    std::vector<double> float_sequence = {1.5, 2.5, 2.0, 3.5};
    auto encoded = ChunkCompressor<double>::delta_encode(float_sequence);
    auto decoded = ChunkCompressor<double>::delta_decode(encoded);

    for (size_t i = 0; i < float_sequence.size(); ++i) {
        EXPECT_DOUBLE_EQ(decoded[i], float_sequence[i]);
    }
}

// Performance Tests
TEST_F(ChunkCompressorTest, LargeSequenceCompression) {
    std::vector<int> large_sequence(10000, 42); // Long sequence of same value
    auto encoded = ChunkCompressor<int>::run_length_encode(large_sequence);
    EXPECT_EQ(encoded.size(), 1);
    EXPECT_TRUE(ComparePairs(encoded[0], std::make_pair(42, size_t(10000))));
}

TEST_F(ChunkCompressorTest, LongDeltaSequence) {
    std::vector<int> long_sequence(10000);
    std::iota(long_sequence.begin(), long_sequence.end(), 0); // 0,1,2,3,...
    auto encoded = ChunkCompressor<int>::delta_encode(long_sequence);
    auto decoded = ChunkCompressor<int>::delta_decode(encoded);
    EXPECT_EQ(decoded, long_sequence);
}