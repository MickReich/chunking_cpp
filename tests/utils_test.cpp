#include "../utils.hpp"
#include <gtest/gtest.h>

using namespace chunk_utils;

TEST(StatisticsTest, BasicStats) {
  std::vector<double> data{1.0, 2.0, 2.0, 3.0, 4.0, 5.0};

  EXPECT_DOUBLE_EQ(Statistics<double>::mean(data), 2.8333333333333335);
  EXPECT_DOUBLE_EQ(Statistics<double>::median(data), 2.5);

  auto [mode_val, mode_freq] = Statistics<double>::mode(data);
  EXPECT_DOUBLE_EQ(mode_val, 2.0);
  EXPECT_EQ(mode_freq, 2);
}

TEST(StatisticsTest, MedianCalculation) {
  // Test odd number of elements
  std::vector<double> odd_data{1.0, 2.0, 3.0};
  EXPECT_DOUBLE_EQ(Statistics<double>::median(odd_data), 2.0);

  // Test even number of elements
  std::vector<double> even_data{1.0, 2.0, 3.0, 4.0};
  EXPECT_DOUBLE_EQ(Statistics<double>::median(even_data), 2.5);

  // Test single element
  std::vector<double> single_data{1.0};
  EXPECT_DOUBLE_EQ(Statistics<double>::median(single_data), 1.0);

  // Test empty vector
  std::vector<double> empty_data;
  EXPECT_DOUBLE_EQ(Statistics<double>::median(empty_data), 0.0);
}

TEST(ChunkManipulatorTest, Operations) {
  auto chunks1 = std::vector<std::vector<int>>{{1, 2}, {3, 4}};
  auto chunks2 = std::vector<std::vector<int>>{{5, 6}};

  auto merged = ChunkManipulator<int>::merge_chunks(chunks1, chunks2);
  EXPECT_EQ(merged.size(), 3);

  auto filtered = ChunkManipulator<int>::filter_chunks(
      merged, [](const auto &chunk) { return chunk[0] > 3; });
  EXPECT_EQ(filtered.size(), 1);
  EXPECT_EQ(filtered[0][0], 5);

  auto transformed =
      ChunkManipulator<int>::transform_chunks(chunks1, [](const auto &chunk) {
        std::vector<int> result = chunk;
        for (auto &val : result)
          val *= 2;
        return result;
      });
  EXPECT_EQ(transformed[0][0], 2);
  EXPECT_EQ(transformed[1][1], 8);
}

TEST(ChunkGeneratorTest, RandomGeneration) {
  auto chunks = ChunkGenerator<double>::generate_random_chunks(3, 2, 0.0, 1.0);
  EXPECT_EQ(chunks.size(), 3);
  EXPECT_EQ(chunks[0].size(), 2);

  for (const auto &chunk : chunks) {
    for (double val : chunk) {
      EXPECT_GE(val, 0.0);
      EXPECT_LE(val, 1.0);
    }
  }
}