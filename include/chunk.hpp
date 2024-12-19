#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <stdexcept>
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
};

#endif // CHUNK_HPP