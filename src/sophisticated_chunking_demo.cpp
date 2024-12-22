#include "sophisticated_chunking.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace sophisticated_chunking;

/**
 * @brief Demonstrate wavelet-based chunking with different data types
 */
void demonstrate_wavelet_chunking() {
    std::cout << "\n=== Wavelet-based Chunking Demonstration ===\n";

    // Numeric data
    std::vector<double> numeric_data = {1.0, 1.1, 1.2, 5.0, 5.1, 5.2, 2.0, 2.1, 2.2};
    WaveletChunking<double> wavelet_numeric(4, 0.5);
    auto numeric_chunks = wavelet_numeric.chunk(numeric_data);

    // Text data
    std::string text = "This is a test sentence. Another sentence here. And one more.";
    std::vector<char> char_data(text.begin(), text.end());
    WaveletChunking<char> wavelet_text(8, 0.3);
    auto text_chunks = wavelet_text.chunk(char_data);

    // Print results
    std::cout << "Numeric chunks: " << numeric_chunks.size() << "\n";
    std::cout << "Text chunks: " << text_chunks.size() << "\n";
}

/**
 * @brief Demonstrate mutual information based chunking
 */
void demonstrate_mutual_information_chunking() {
    std::cout << "\n=== Mutual Information Chunking Demonstration ===\n";

    // Word-level data
    std::vector<std::string> words = {"the", "quick", "brown", "fox", "jumps", 
                                     "over", "the", "lazy", "dog"};
    MutualInformationChunking<std::string> mi_words(3, 0.4);
    auto word_chunks = mi_words.chunk(words);

    // Binary data
    std::vector<uint8_t> binary_data = {0xFF, 0xFE, 0xFD, 0x00, 0x01, 0x02};
    MutualInformationChunking<uint8_t> mi_binary(2, 0.5);
    auto binary_chunks = mi_binary.chunk(binary_data);

    // Print results
    std::cout << "Word chunks: " << word_chunks.size() << "\n";
    std::cout << "Binary chunks: " << binary_chunks.size() << "\n";
}

/**
 * @brief Demonstrate DTW-based chunking
 */
void demonstrate_dtw_chunking() {
    std::cout << "\n=== DTW-based Chunking Demonstration ===\n";

    // Time series data
    std::vector<float> time_series = {1.0f, 1.2f, 1.1f, 5.0f, 5.2f, 5.1f, 
                                     2.0f, 2.2f, 2.1f};
    DTWChunking<float> dtw_numeric(5, 1.5);
    auto ts_chunks = dtw_numeric.chunk(time_series);

    // Categorical data
    std::vector<char> categorical = {'A', 'A', 'B', 'B', 'C', 'C', 'A', 'B'};
    DTWChunking<char> dtw_categorical(3, 1.0);
    auto cat_chunks = dtw_categorical.chunk(categorical);

    // Print results
    std::cout << "Time series chunks: " << ts_chunks.size() << "\n";
    std::cout << "Categorical chunks: " << cat_chunks.size() << "\n";
}

int main() {
    demonstrate_wavelet_chunking();
    demonstrate_mutual_information_chunking();
    demonstrate_dtw_chunking();
    return 0;
} 