#include "sophisticated_chunking.hpp"
#include "gtest/gtest.h"
#include <random>
#include <string>
#include <vector>

using namespace sophisticated_chunking;

class WaveletChunkingTest : public ::testing::Test {
protected:
    void SetUp() override {
        numeric_data = {1.0, 1.1, 1.2, 5.0, 5.1, 5.2, 2.0, 2.1, 2.2};
        text_data = std::vector<char>{'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    }

    std::vector<double> numeric_data;
    std::vector<char> text_data;
};

class MutualInformationChunkingTest : public ::testing::Test {
protected:
    void SetUp() override {
        word_data = {"the", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog"};
        binary_data = {0xFF, 0xFE, 0xFD, 0x00, 0x01, 0x02};
    }

    std::vector<std::string> word_data;
    std::vector<uint8_t> binary_data;
};

class DTWChunkingTest : public ::testing::Test {
protected:
    void SetUp() override {
        time_series = {1.0f, 1.2f, 1.1f, 5.0f, 5.2f, 5.1f, 2.0f, 2.2f, 2.1f};
        categorical = {'A', 'A', 'B', 'B', 'C', 'C', 'A', 'B'};
    }

    std::vector<float> time_series;
    std::vector<char> categorical;
};

TEST_F(WaveletChunkingTest, NumericDataChunking) {
    WaveletChunking<double> chunker(4, 0.5);
    auto chunks = chunker.chunk(numeric_data);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}

TEST_F(WaveletChunkingTest, TextDataChunking) {
    WaveletChunking<char> chunker(8, 0.3);
    auto chunks = chunker.chunk(text_data);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}

TEST_F(MutualInformationChunkingTest, WordLevelChunking) {
    MutualInformationChunking<std::string> chunker(3, 0.4);
    auto chunks = chunker.chunk(word_data);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}

TEST_F(MutualInformationChunkingTest, BinaryDataChunking) {
    MutualInformationChunking<uint8_t> chunker(2, 0.5);
    auto chunks = chunker.chunk(binary_data);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}

TEST_F(DTWChunkingTest, TimeSeriesChunking) {
    DTWChunking<float> chunker(5, 1.5);
    auto chunks = chunker.chunk(time_series);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}

TEST_F(DTWChunkingTest, CategoricalDataChunking) {
    DTWChunking<char> chunker(3, 1.0);
    auto chunks = chunker.chunk(categorical);

    EXPECT_GT(chunks.size(), 0);
    for (const auto& chunk : chunks) {
        EXPECT_GT(chunk.size(), 0);
    }
}