#include <iostream>
#include <iomanip>
#include <vector>
#include "../include/chunk.hpp"
#include "../config.hpp"
#include "../data_structures.hpp"
#include "../utils.hpp"
#include <iomanip>
#include <iostream>
#include <vector>


// Helper function to print chunks
template <typename T>
void print_chunks(const std::vector<std::vector<T>> &chunks) {
  std::cout << "Chunks: [" << std::endl;
  for (size_t i = 0; i < chunks.size(); ++i) {
    std::cout << "  " << i << ": [";
    for (const auto &value : chunks[i]) {
      std::cout << std::fixed << std::setprecision(2) << value << " ";
    }
    std::cout << "]" << std::endl;
  }
  std::cout << "]" << std::endl;
}

int main() {
  // Example 1: Integer chunking
  std::cout << "\n=== Integer Chunking Example ===" << std::endl;
  Chunk<int> int_chunker(4); // Chunks of size 4
  std::vector<int> int_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int_chunker.add(int_data);

  std::cout << "Total elements: " << int_chunker.size() << std::endl;
  std::cout << "Number of chunks: " << int_chunker.chunk_count() << std::endl;
  print_chunks(int_chunker.get_chunks());

  // Example 2: Float chunking with partial chunk
  std::cout << "\n=== Float Chunking Example ===" << std::endl;
  Chunk<float> float_chunker(3); // Chunks of size 3
  float_chunker.add({1.1f, 2.2f, 3.3f, 4.4f, 5.5f});
  print_chunks(float_chunker.get_chunks());

  // Example 3: Double chunking with individual additions
  std::cout << "\n=== Double Chunking Example ===" << std::endl;
  Chunk<double> double_chunker(2); // Chunks of size 2
  double_chunker.add(1.5);
  double_chunker.add(2.5);
  double_chunker.add(3.5);

  std::cout << "Getting specific chunks:" << std::endl;
  auto chunk0 = double_chunker.get_chunk(0);
  std::cout << "Chunk 0: [" << chunk0[0] << " " << chunk0[1] << "]"
            << std::endl;

  auto chunk1 = double_chunker.get_chunk(1);
  std::cout << "Chunk 1: [" << chunk1[0] << "]" << std::endl;

  // Example 4: Error handling
  std::cout << "\n=== Error Handling Example ===" << std::endl;
  try {
    Chunk<int> invalid_chunker(0); // Should throw exception
  } catch (const std::invalid_argument &e) {
    std::cout << "Expected error: " << e.what() << std::endl;
  }

  // Example 5: Overlapping chunks
  std::cout << "\n=== Overlapping Chunks Example ===" << std::endl;
  Chunk<int> overlap_chunker(4);
  std::vector<int> overlap_data = {1, 2, 3, 4, 5, 6, 7, 8};
  overlap_chunker.add(overlap_data);

  std::cout << "Chunks with 2-element overlap:" << std::endl;
  auto overlapping_chunks = overlap_chunker.get_overlapping_chunks(2);
  print_chunks(overlapping_chunks);

  // Example 6: Chunking by predicate (start new chunk on even numbers)
  std::cout << "\n=== Predicate-based Chunking Example ===" << std::endl;
  Chunk<int> pred_chunker(
      1); // Use minimum chunk size of 1 for predicate-based chunking
  std::vector<int> pred_data = {1, 2, 3, 4, 5, 6, 7, 8};
  pred_chunker.add(pred_data);

  auto predicate_chunks =
      pred_chunker.chunk_by_predicate([](int x) { return x % 2 == 0; });
  std::cout << "Chunks starting with even numbers:" << std::endl;
  print_chunks(predicate_chunks);

  // Example 7: Chunking by sum
  std::cout << "\n=== Sum-based Chunking Example ===" << std::endl;
  Chunk<int> sum_chunker(
      1); // Use minimum chunk size of 1 for sum-based chunking
  std::vector<int> sum_data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  sum_chunker.add(sum_data);

  auto sum_chunks = sum_chunker.chunk_by_sum(10);
  std::cout << "Chunks with sum <= 10:" << std::endl;
  print_chunks(sum_chunks);

  // Example 8: Equal division chunking
  std::cout << "\n=== Equal Division Chunking Example ===" << std::endl;
  Chunk<int> equal_chunker(
      1); // Use minimum chunk size of 1 for n-division chunking
  std::vector<int> equal_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  equal_chunker.add(equal_data);

  auto equal_chunks = equal_chunker.chunk_into_n(3);
  std::cout << "Data divided into 3 chunks:" << std::endl;
  print_chunks(equal_chunks);

  // Example 9: Using data structures
  std::cout << "\n=== Data Structures Example ===" << std::endl;

  // Circular Buffer example
  CircularBuffer<int> buffer(3);
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);
  buffer.push(4); // This will overwrite 1
  std::cout << "Circular Buffer: ";
  for (const auto &val : buffer.to_vector()) {
    std::cout << val << " ";
  }
  std::cout << std::endl;

  // Sliding Window example
  SlidingWindow<double> window(3);
  window.push(1.0);
  window.push(2.0);
  window.push(3.0);
  window.push(4.0); // This will remove 1.0
  std::cout << "Sliding Window Average: " << window.average() << std::endl;

  // ChunkList example
  ChunkList<int> chunk_list;
  chunk_list.append_chunk({1, 2, 3});
  chunk_list.append_chunk({4, 5, 6});
  chunk_list.prepend_chunk({-1, 0});
  std::cout << "ChunkList flattened: ";
  for (const auto &val : chunk_list.flatten()) {
    std::cout << val << " ";
  }
  std::cout << std::endl;

  // Example 10: Using Statistics
  std::cout << "\n=== Statistics Example ===" << std::endl;
  std::vector<double> stats_data = {1.0, 2.0, 2.0, 3.0, 4.0, 5.0};
  std::cout << "Mean: " << chunk_utils::Statistics<double>::mean(stats_data)
            << std::endl;
  std::cout << "Median: " << chunk_utils::Statistics<double>::median(stats_data)
            << std::endl;
  auto [mode_val, mode_freq] =
      chunk_utils::Statistics<double>::mode(stats_data);
  std::cout << "Mode: " << mode_val << " (frequency: " << mode_freq << ")"
            << std::endl;

  // Example 11: Chunk Manipulation
  std::cout << "\n=== Chunk Manipulation Example ===" << std::endl;
  auto chunks1 = std::vector<std::vector<int>>{{1, 2}, {3, 4}};
  auto chunks2 = std::vector<std::vector<int>>{{5, 6}, {7, 8}};

  auto merged =
      chunk_utils::ChunkManipulator<int>::merge_chunks(chunks1, chunks2);
  std::cout << "Merged chunks:" << std::endl;
  print_chunks(merged);

  auto filtered = chunk_utils::ChunkManipulator<int>::filter_chunks(
      merged, [](const auto &chunk) { return chunk[0] > 3; });
  std::cout << "Filtered chunks (first element > 3):" << std::endl;
  print_chunks(filtered);

  // Example 12: Random Chunk Generation
  std::cout << "\n=== Random Chunk Generation Example ===" << std::endl;
  auto random_chunks =
      chunk_utils::ChunkGenerator<double>::generate_random_chunks(3, 4, 0.0,
                                                                  10.0);
  std::cout << "Randomly generated chunks:" << std::endl;
  print_chunks(random_chunks);

  return 0;
}