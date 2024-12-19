#include <iostream>
#include <iomanip>
#include <vector>
#include "chunk.hpp"

// Helper function to print chunks
template<typename T>
void print_chunks(const std::vector<std::vector<T>>& chunks) {
    std::cout << "Chunks: [" << std::endl;
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << "  " << i << ": [";
        for (const auto& value : chunks[i]) {
            std::cout << std::fixed << std::setprecision(2) << value << " ";
        }
        std::cout << "]" << std::endl;
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Example 1: Integer chunking
    std::cout << "\n=== Integer Chunking Example ===" << std::endl;
    Chunk<int> int_chunker(4);  // Chunks of size 4
    std::vector<int> int_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int_chunker.add(int_data);
    
    std::cout << "Total elements: " << int_chunker.size() << std::endl;
    std::cout << "Number of chunks: " << int_chunker.chunk_count() << std::endl;
    print_chunks(int_chunker.get_chunks());

    // Example 2: Float chunking with partial chunk
    std::cout << "\n=== Float Chunking Example ===" << std::endl;
    Chunk<float> float_chunker(3);  // Chunks of size 3
    float_chunker.add({1.1f, 2.2f, 3.3f, 4.4f, 5.5f});
    print_chunks(float_chunker.get_chunks());

    // Example 3: Double chunking with individual additions
    std::cout << "\n=== Double Chunking Example ===" << std::endl;
    Chunk<double> double_chunker(2);  // Chunks of size 2
    double_chunker.add(1.5);
    double_chunker.add(2.5);
    double_chunker.add(3.5);
    
    std::cout << "Getting specific chunks:" << std::endl;
    auto chunk0 = double_chunker.get_chunk(0);
    std::cout << "Chunk 0: [" << chunk0[0] << " " << chunk0[1] << "]" << std::endl;
    
    auto chunk1 = double_chunker.get_chunk(1);
    std::cout << "Chunk 1: [" << chunk1[0] << "]" << std::endl;

    // Example 4: Error handling
    std::cout << "\n=== Error Handling Example ===" << std::endl;
    try {
        Chunk<int> invalid_chunker(0);  // Should throw exception
    } catch (const std::invalid_argument& e) {
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
    Chunk<int> pred_chunker(0);  // chunk_size doesn't matter for predicate-based chunking
    std::vector<int> pred_data = {1, 2, 3, 4, 5, 6, 7, 8};
    pred_chunker.add(pred_data);
    
    auto predicate_chunks = pred_chunker.chunk_by_predicate([](int x) { return x % 2 == 0; });
    std::cout << "Chunks starting with even numbers:" << std::endl;
    print_chunks(predicate_chunks);

    // Example 7: Chunking by sum
    std::cout << "\n=== Sum-based Chunking Example ===" << std::endl;
    Chunk<int> sum_chunker(0);  // chunk_size doesn't matter for sum-based chunking
    std::vector<int> sum_data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    sum_chunker.add(sum_data);
    
    auto sum_chunks = sum_chunker.chunk_by_sum(10);
    std::cout << "Chunks with sum <= 10:" << std::endl;
    print_chunks(sum_chunks);

    // Example 8: Equal division chunking
    std::cout << "\n=== Equal Division Chunking Example ===" << std::endl;
    Chunk<int> equal_chunker(0);  // chunk_size doesn't matter for n-division chunking
    std::vector<int> equal_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    equal_chunker.add(equal_data);
    
    auto equal_chunks = equal_chunker.chunk_into_n(3);
    std::cout << "Data divided into 3 chunks:" << std::endl;
    print_chunks(equal_chunks);

    return 0;
} 