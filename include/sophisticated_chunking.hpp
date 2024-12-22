#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <memory>
#include <random>
#include <type_traits>

namespace sophisticated_chunking {

/**
 * @brief Wavelet-based chunking strategy using signal processing principles
 * @tparam T The type of elements to be chunked
 */
template<typename T>
class WaveletChunking {
private:
    size_t window_size_;
    double threshold_;

    /**
     * @brief Compute discrete wavelet transform coefficients
     * @param data Input data sequence
     * @return Vector of wavelet coefficients
     */
    std::vector<double> computeWaveletCoefficients(const std::vector<T>& data) const;

public:
    /**
     * @brief Constructor for wavelet-based chunking
     * @param window_size Size of the sliding window
     * @param threshold Coefficient threshold for chunk boundaries
     */
    WaveletChunking(size_t window_size = 8, double threshold = 0.5)
        : window_size_(window_size), threshold_(threshold) {}

    /**
     * @brief Chunk data based on wavelet transform analysis
     * @param data Input data to be chunked
     * @return Vector of chunks
     */
    std::vector<std::vector<T>> chunk(const std::vector<T>& data) const;
};

/**
 * @brief Information theory based chunking using mutual information
 * @tparam T The type of elements to be chunked
 */
template<typename T>
class MutualInformationChunking {
private:
    size_t context_size_;
    double mi_threshold_;

    /**
     * @brief Calculate mutual information between adjacent segments
     * @param segment1 First segment
     * @param segment2 Second segment
     * @return Mutual information value
     */
    double calculateMutualInformation(const std::vector<T>& segment1, 
                                    const std::vector<T>& segment2) const;

public:
    /**
     * @brief Constructor for mutual information based chunking
     * @param context_size Size of context window
     * @param mi_threshold Threshold for mutual information
     */
    MutualInformationChunking(size_t context_size = 5, double mi_threshold = 0.3)
        : context_size_(context_size), mi_threshold_(mi_threshold) {}

    /**
     * @brief Chunk data based on mutual information analysis
     * @param data Input data to be chunked
     * @return Vector of chunks
     */
    std::vector<std::vector<T>> chunk(const std::vector<T>& data) const;
};

/**
 * @brief Dynamic time warping based chunking for sequence alignment
 * @tparam T The type of elements to be chunked
 */
template<typename T>
class DTWChunking {
private:
    size_t window_size_;
    double dtw_threshold_;

    /**
     * @brief Compute DTW distance between sequences
     * @param seq1 First sequence
     * @param seq2 Second sequence
     * @return DTW distance
     */
    double computeDTWDistance(const std::vector<T>& seq1, 
                            const std::vector<T>& seq2) const;

public:
    /**
     * @brief Constructor for DTW-based chunking
     * @param window_size Size of the warping window
     * @param dtw_threshold Threshold for chunk boundaries
     */
    DTWChunking(size_t window_size = 10, double dtw_threshold = 2.0)
        : window_size_(window_size), dtw_threshold_(dtw_threshold) {}

    /**
     * @brief Chunk data based on DTW analysis
     * @param data Input data to be chunked
     * @return Vector of chunks
     */
    std::vector<std::vector<T>> chunk(const std::vector<T>& data) const;
};

template<typename T>
std::vector<double> WaveletChunking<T>::computeWaveletCoefficients(const std::vector<T>& data) const {
    if (data.size() < window_size_) {
        return std::vector<double>();
    }

    std::vector<double> coefficients;
    coefficients.reserve(data.size() - window_size_ + 1);

    // Haar wavelet transform
    for (size_t i = 0; i <= data.size() - window_size_; ++i) {
        double sum = 0.0;
        for (size_t j = 0; j < window_size_ / 2; ++j) {
            double diff = static_cast<double>(data[i + j]) - 
                         static_cast<double>(data[i + window_size_ - 1 - j]);
            sum += diff * diff;
        }
        coefficients.push_back(std::sqrt(sum / window_size_));
    }

    return coefficients;
}

template<typename T>
std::vector<std::vector<T>> WaveletChunking<T>::chunk(const std::vector<T>& data) const {
    if (data.empty()) {
        return {};
    }

    auto coefficients = computeWaveletCoefficients(data);
    std::vector<std::vector<T>> chunks;
    std::vector<T> current_chunk;

    size_t i = 0;
    for (const T& value : data) {
        current_chunk.push_back(value);
        
        if (i < coefficients.size() && coefficients[i] > threshold_) {
            if (!current_chunk.empty()) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
            }
        }
        ++i;
    }

    if (!current_chunk.empty()) {
        chunks.push_back(current_chunk);
    }

    return chunks;
}

template<typename T>
double MutualInformationChunking<T>::calculateMutualInformation(
    const std::vector<T>& segment1, const std::vector<T>& segment2) const {
    if (segment1.empty() || segment2.empty()) {
        return 0.0;
    }

    // Calculate frequency distributions
    std::map<T, double> p1, p2;
    std::map<std::pair<T, T>, double> p12;
    
    for (const auto& val : segment1) {
        p1[val] += 1.0 / segment1.size();
    }
    
    for (const auto& val : segment2) {
        p2[val] += 1.0 / segment2.size();
    }

    // Calculate joint distribution
    size_t min_size = std::min(segment1.size(), segment2.size());
    for (size_t i = 0; i < min_size; ++i) {
        p12[{segment1[i], segment2[i]}] += 1.0 / min_size;
    }

    // Calculate mutual information
    double mi = 0.0;
    for (const auto& [val1, prob1] : p1) {
        for (const auto& [val2, prob2] : p2) {
            auto joint_prob = p12[{val1, val2}];
            if (joint_prob > 0) {
                mi += joint_prob * std::log2(joint_prob / (prob1 * prob2));
            }
        }
    }

    return mi;
}

template<typename T>
std::vector<std::vector<T>> MutualInformationChunking<T>::chunk(const std::vector<T>& data) const {
    if (data.size() < 2 * context_size_) {
        return {data};
    }

    std::vector<std::vector<T>> chunks;
    std::vector<T> current_chunk;
    
    for (size_t i = 0; i < data.size(); ++i) {
        current_chunk.push_back(data[i]);
        
        if (current_chunk.size() >= context_size_ && i + context_size_ < data.size()) {
            std::vector<T> next_segment(data.begin() + i + 1, 
                                      data.begin() + std::min(i + 1 + context_size_, data.size()));
            
            double mi = calculateMutualInformation(current_chunk, next_segment);
            
            if (mi < mi_threshold_) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
            }
        }
    }
    
    if (!current_chunk.empty()) {
        chunks.push_back(current_chunk);
    }
    
    return chunks;
}

template<typename T>
double DTWChunking<T>::computeDTWDistance(const std::vector<T>& seq1, 
                                         const std::vector<T>& seq2) const {
    if (seq1.empty() || seq2.empty()) {
        return std::numeric_limits<double>::infinity();
    }

    // Initialize DTW matrix
    std::vector<std::vector<double>> dtw(seq1.size() + 1, 
                                        std::vector<double>(seq2.size() + 1, 
                                        std::numeric_limits<double>::infinity()));
    dtw[0][0] = 0.0;

    // Fill DTW matrix
    for (size_t i = 1; i <= seq1.size(); ++i) {
        for (size_t j = std::max(1ul, i - window_size_); 
             j <= std::min(seq2.size(), i + window_size_); ++j) {
            double cost = std::abs(static_cast<double>(seq1[i-1]) - static_cast<double>(seq2[j-1]));
            dtw[i][j] = cost + std::min({
                dtw[i-1][j],      // insertion
                dtw[i][j-1],      // deletion
                dtw[i-1][j-1]     // match
            });
        }
    }

    return dtw[seq1.size()][seq2.size()];
}

template<typename T>
std::vector<std::vector<T>> DTWChunking<T>::chunk(const std::vector<T>& data) const {
    if (data.size() < 2 * window_size_) {
        return {data};
    }

    std::vector<std::vector<T>> chunks;
    std::vector<T> current_chunk;
    
    for (size_t i = 0; i < data.size(); ++i) {
        current_chunk.push_back(data[i]);
        
        if (current_chunk.size() >= window_size_ && i + window_size_ < data.size()) {
            std::vector<T> next_window(data.begin() + i + 1, 
                                     data.begin() + i + 1 + window_size_);
            
            double distance = computeDTWDistance(
                std::vector<T>(current_chunk.end() - window_size_, current_chunk.end()),
                next_window
            );
            
            if (distance > dtw_threshold_) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
            }
        }
    }
    
    if (!current_chunk.empty()) {
        chunks.push_back(current_chunk);
    }
    
    return chunks;
}

} // namespace sophisticated_chunking 