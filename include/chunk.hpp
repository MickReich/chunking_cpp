#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "config.hpp"

/**
 * @file chunk.hpp
 * @brief Template class for chunk-based data processing
 * @author Jonathan Reich
 * @date 2024-12-19
 */

/**
 * @brief A template class for managing and processing data in chunks
 *
 * This class provides various methods for splitting data into chunks,
 * including fixed-size chunks, overlapping chunks, and predicate-based chunking.
 *
 * @tparam T The type of elements to be chunked
 */
template <typename T>
class Chunk {
private:
    std::vector<T> data_;
    size_t chunk_size_;

    // Helper function for validation
    void validate_size(size_t size, const std::string& param) const {
        if (size == 0) {
            throw std::invalid_argument(param + " must be greater than 0");
        }
    }

public:
    // Constructor with chunk size
    explicit Chunk(size_t chunk_size) : chunk_size_(chunk_size) {
        if (chunk_size == 0) {
            throw std::invalid_argument("Chunk size must be greater than 0");
        }
    }

    // Add data to the chunk
    void add(const T& value) {
        data_.push_back(value);
    }

    // Add a vector of data
    void add(const std::vector<T>& values) {
        data_.insert(data_.end(), values.begin(), values.end());
    }

    // Get chunks as vector of vectors
    std::vector<std::vector<T>> get_chunks() const {
        std::vector<std::vector<T>> chunks;

        for (size_t i = 0; i < data_.size(); i += chunk_size_) {
            std::vector<T> chunk;
            size_t end = std::min(i + chunk_size_, data_.size());
            chunk.insert(chunk.end(), data_.begin() + i, data_.begin() + end);
            chunks.push_back(chunk);
        }

        return chunks;
    }

    // Get a specific chunk by index
    std::vector<T> get_chunk(size_t index) const {
        size_t start = index * chunk_size_;
        if (start >= data_.size()) {
            throw std::out_of_range("Chunk index out of range");
        }

        size_t end = std::min(start + chunk_size_, data_.size());
        return std::vector<T>(data_.begin() + start, data_.begin() + end);
    }

    // Get the number of chunks
    size_t chunk_count() const {
        return (data_.size() + chunk_size_ - 1) / chunk_size_;
    }

    // Get the chunk size
    size_t chunk_size() const {
        return chunk_size_;
    }

    // Get total size of data
    size_t size() const {
        return data_.size();
    }

    // Clear all data
    void clear() {
        data_.clear();
    }

    // Check if empty
    bool empty() const {
        return data_.empty();
    }

    // New chunking strategies

    // Chunk with overlap - each chunk shares n elements with the next chunk
    std::vector<std::vector<T>> get_overlapping_chunks(size_t overlap) const {
        if (overlap >= chunk_size_) {
            throw std::invalid_argument("Overlap must be less than chunk size");
        }

        std::vector<std::vector<T>> chunks;
        size_t step = chunk_size_ - overlap;

        for (size_t i = 0; i + chunk_size_ <= data_.size(); i += step) {
            chunks.push_back(std::vector<T>(data_.begin() + i, data_.begin() + i + chunk_size_));
        }

        // Handle remaining elements if any
        if (data_.size() % step != 0) {
            size_t remaining = data_.size() % step;
            if (remaining > overlap) {
                chunks.push_back(std::vector<T>(data_.end() - chunk_size_, data_.end()));
            }
        }

        return chunks;
    }

    // Chunk by predicate - start new chunk when predicate returns true
    // Note: This method ignores the original chunk_size_
    template <typename Pred>
    std::vector<std::vector<T>> chunk_by_predicate(Pred predicate) const {
        std::vector<std::vector<T>> chunks;
        if (data_.empty())
            return chunks;

        std::vector<T> current_chunk;
        current_chunk.push_back(data_[0]); // Add first element

        for (size_t i = 1; i < data_.size(); ++i) {
            if (predicate(data_[i])) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
            }
            current_chunk.push_back(data_[i]);
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }

    // Chunk by sum - create chunks that sum up to a target value
    // Note: This method ignores the original chunk_size_
    std::vector<std::vector<T>> chunk_by_sum(T target_sum) const {
        std::vector<std::vector<T>> chunks;
        if (data_.empty())
            return chunks;

        std::vector<T> current_chunk;
        T current_sum = T();

        for (const T& item : data_) {
            if (current_sum + item > target_sum && !current_chunk.empty()) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
                current_sum = T();
            }
            current_chunk.push_back(item);
            current_sum += item;
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }

    // Chunk into n equal(ish) parts
    // Note: This method ignores the original chunk_size_
    std::vector<std::vector<T>> chunk_into_n(size_t n) const {
        if (n == 0) {
            throw std::invalid_argument("Number of chunks must be greater than 0");
        }
        if (data_.empty())
            return std::vector<std::vector<T>>();

        n = std::min(n, data_.size());
        std::vector<std::vector<T>> chunks(n);
        size_t base_size = data_.size() / n;
        size_t remainder = data_.size() % n;

        size_t current_pos = 0;
        for (size_t i = 0; i < n; ++i) {
            size_t current_chunk_size = base_size + (i < remainder ? 1 : 0);
            chunks[i] = std::vector<T>(data_.begin() + current_pos,
                                       data_.begin() + current_pos + current_chunk_size);
            current_pos += current_chunk_size;
        }

        return chunks;
    }

    // Additional chunking strategies

    /**
     * @brief Chunk data based on a moving window
     * @param window_size Size of the sliding window
     * @param step Size of step between windows
     * @return Vector of chunks representing sliding windows
     */
    std::vector<std::vector<T>> sliding_window(size_t window_size, size_t step = 1) const {
        validate_size(window_size, "Window size");
        validate_size(step, "Step size");

        std::vector<std::vector<T>> chunks;
        for (size_t i = 0; i + window_size <= data_.size(); i += step) {
            chunks.push_back(std::vector<T>(data_.begin() + i, data_.begin() + i + window_size));
        }
        return chunks;
    }

    /**
     * @brief Chunk data based on a statistical threshold
     * @param threshold The statistical threshold for creating new chunks
     * @param stat_func Function to compute the statistic (e.g., mean, median)
     * @return Vector of chunks divided by statistical threshold
     */
    template <typename StatFunc>
    std::vector<std::vector<T>> chunk_by_statistic(T threshold, StatFunc stat_func) const {
        std::vector<std::vector<T>> chunks;
        if (data_.empty())
            return chunks;

        std::vector<T> current_chunk;
        current_chunk.push_back(data_[0]);

        for (size_t i = 1; i < data_.size(); ++i) {
            if (stat_func(current_chunk) > threshold) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
            }
            current_chunk.push_back(data_[i]);
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }

    /**
     * @brief Chunk data into groups with similar values
     * @param similarity_threshold Maximum difference between values in a chunk
     * @return Vector of chunks with similar values
     */
    std::vector<std::vector<T>> chunk_by_similarity(T similarity_threshold) const {
        std::vector<std::vector<T>> chunks;
        if (data_.empty())
            return chunks;

        std::vector<T> current_chunk{data_[0]};
        T chunk_mean = data_[0];

        for (size_t i = 1; i < data_.size(); ++i) {
            if (std::abs(data_[i] - chunk_mean) > similarity_threshold) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
                chunk_mean = data_[i];
            }
            current_chunk.push_back(data_[i]);
            chunk_mean = std::accumulate(current_chunk.begin(), current_chunk.end(), T(0)) /
                         static_cast<T>(current_chunk.size());
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }

    /**
     * @brief Chunk data based on monotonicity changes
     * @return Vector of chunks where each chunk is monotonic
     */
    std::vector<std::vector<T>> chunk_by_monotonicity() const {
        std::vector<std::vector<T>> chunks;
        if (data_.size() < 2)
            return {data_};

        std::vector<T> current_chunk{data_[0]};
        bool increasing = data_[1] > data_[0];

        for (size_t i = 1; i < data_.size(); ++i) {
            if ((data_[i] > data_[i - 1]) != increasing) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
                if (i < data_.size() - 1) {
                    increasing = data_[i + 1] > data_[i];
                }
            }
            current_chunk.push_back(data_[i]);
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }

    /**
     * @brief Chunk data into fixed-size chunks with padding
     * @param pad_value Value to use for padding incomplete chunks
     * @return Vector of equal-sized chunks with padding
     */
    std::vector<std::vector<T>> get_padded_chunks(const T& pad_value) const {
        std::vector<std::vector<T>> chunks;

        for (size_t i = 0; i < data_.size(); i += chunk_size_) {
            std::vector<T> chunk;
            size_t remaining = std::min(chunk_size_, data_.size() - i);
            chunk.insert(chunk.end(), data_.begin() + i, data_.begin() + i + remaining);

            // Pad if necessary
            if (remaining < chunk_size_) {
                chunk.insert(chunk.end(), chunk_size_ - remaining, pad_value);
            }

            chunks.push_back(chunk);
        }

        return chunks;
    }
};

#endif // CHUNK_HPP