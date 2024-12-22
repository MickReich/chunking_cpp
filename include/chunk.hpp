#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include "config.hpp"

/**
 * @file chunk.hpp
 * @brief Defines the Chunk class for managing data in fixed-size chunks.
 */

#pragma once

/**
 * @brief A class for managing data in fixed-size chunks.
 * @tparam T The type of elements stored in the chunk.
 */
template <typename T>
class Chunk {
private:
    size_t chunk_size;
    std::vector<T> data;
    std::vector<std::vector<T>> chunks;

    // Helper function for validation
    void validate_size(size_t size, const std::string& param) const {
        if (size == 0) {
            throw std::invalid_argument(param + " must be greater than 0");
        }
    }

public:
    /**
     * @brief Constructs a Chunk with a specified size.
     * @param size The size of each chunk.
     * @throws std::invalid_argument if size is zero.
     */
    explicit Chunk(size_t size) : chunk_size(size) {
        if (size == 0) {
            throw std::invalid_argument("Chunk size must be greater than 0");
        }
    }

    size_t size() const {
        return data.size();
    }

    size_t chunk_count() const {
        return (data.size() + chunk_size - 1) / chunk_size;
    }

    /**
     * @brief Adds a single element to the chunk.
     * @param element The element to add.
     */
    void add(const T& element) {
        data.push_back(element);
        update_chunks();
    }

    /**
     * @brief Adds a vector of elements to the chunk.
     * @param elements The elements to add.
     */
    void add(const std::vector<T>& elements) {
        data.insert(data.end(), elements.begin(), elements.end());
        update_chunks();
    }

    /**
     * @brief Retrieves all chunks.
     * @return A vector of vectors, where each inner vector is a chunk.
     */
    std::vector<std::vector<T>> get_chunks() const {
        return chunks;
    }

    /**
     * @brief Retrieves a specific chunk by index.
     * @param index The index of the chunk to retrieve.
     * @return The chunk at the specified index.
     * @throws std::out_of_range if index is invalid.
     */
    std::vector<T> get_chunk(size_t index) const {
        size_t start = index * chunk_size;
        if (start >= data.size()) {
            throw std::out_of_range("Chunk index out of range");
        }
        size_t end = std::min(start + chunk_size, data.size());
        return std::vector<T>(data.begin() + start, data.begin() + end);
    }

    std::vector<std::vector<T>> get_overlapping_chunks(size_t overlap) const {
        if (overlap >= chunk_size) {
            throw std::invalid_argument("Overlap must be less than chunk size");
        }
        std::vector<std::vector<T>> result;
        size_t step = chunk_size - overlap;
        for (size_t i = 0; i + chunk_size <= data.size(); i += step) {
            result.push_back(std::vector<T>(data.begin() + i, data.begin() + i + chunk_size));
        }
        return result;
    }

    template <typename Pred>
    std::vector<std::vector<T>> chunk_by_predicate(Pred predicate) const {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;
        std::vector<T> current_chunk;
        for (const T& item : data) {
            if (predicate(item) && !current_chunk.empty()) {
                result.push_back(current_chunk);
                current_chunk.clear();
            }
            current_chunk.push_back(item);
        }
        if (!current_chunk.empty()) {
            result.push_back(current_chunk);
        }
        return result;
    }

    std::vector<std::vector<T>> chunk_by_sum(T target_sum) const {
        std::vector<std::vector<T>> result;
        std::vector<T> current_chunk;
        T current_sum = T();
        for (const T& item : data) {
            if (current_sum + item > target_sum && !current_chunk.empty()) {
                result.push_back(current_chunk);
                current_chunk.clear();
                current_sum = T();
            }
            current_chunk.push_back(item);
            current_sum += item;
        }
        if (!current_chunk.empty()) {
            result.push_back(current_chunk);
        }
        return result;
    }

    std::vector<std::vector<T>> chunk_into_n(size_t n) const {
        if (n == 0)
            throw std::invalid_argument("Number of chunks must be greater than 0");
        if (data.empty())
            return std::vector<std::vector<T>>();
        n = std::min(n, data.size());
        std::vector<std::vector<T>> result(n);
        size_t base_size = data.size() / n;
        size_t remainder = data.size() % n;
        size_t pos = 0;
        for (size_t i = 0; i < n; ++i) {
            size_t chunk_size = base_size + (i < remainder ? 1 : 0);
            result[i] = std::vector<T>(data.begin() + pos, data.begin() + pos + chunk_size);
            pos += chunk_size;
        }
        return result;
    }

    std::vector<std::vector<T>> chunk_by_monotonicity() const {
        std::vector<std::vector<T>> result;
        if (data.size() < 2)
            return {data};
        std::vector<T> current_chunk{data[0]};
        bool increasing = data[1] > data[0];
        for (size_t i = 1; i < data.size(); ++i) {
            if ((data[i] > data[i - 1]) != increasing && !current_chunk.empty()) {
                result.push_back(current_chunk);
                current_chunk.clear();
                if (i < data.size() - 1)
                    increasing = data[i + 1] > data[i];
            }
            current_chunk.push_back(data[i]);
        }
        if (!current_chunk.empty())
            result.push_back(current_chunk);
        return result;
    }

    template <typename StatFunc>
    std::vector<std::vector<T>> chunk_by_statistic(T threshold, StatFunc stat_func) const {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;
        std::vector<T> current_chunk{data[0]};
        for (size_t i = 1; i < data.size(); ++i) {
            if (stat_func(current_chunk) > threshold) {
                result.push_back(current_chunk);
                current_chunk.clear();
            }
            current_chunk.push_back(data[i]);
        }
        if (!current_chunk.empty())
            result.push_back(current_chunk);
        return result;
    }

    std::vector<std::vector<T>> chunk_by_similarity(T threshold) const {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;
        std::vector<T> current_chunk{data[0]};
        T chunk_mean = data[0];
        for (size_t i = 1; i < data.size(); ++i) {
            if (std::abs(data[i] - chunk_mean) > threshold) {
                result.push_back(current_chunk);
                current_chunk.clear();
                chunk_mean = data[i];
            }
            current_chunk.push_back(data[i]);
            chunk_mean = std::accumulate(current_chunk.begin(), current_chunk.end(), T(0)) /
                         static_cast<T>(current_chunk.size());
        }
        if (!current_chunk.empty())
            result.push_back(current_chunk);
        return result;
    }

    std::vector<std::vector<T>> get_padded_chunks(const T& pad_value) const {
        std::vector<std::vector<T>> result;
        for (size_t i = 0; i < data.size(); i += chunk_size) {
            std::vector<T> chunk;
            size_t remaining = std::min(chunk_size, data.size() - i);
            chunk.insert(chunk.end(), data.begin() + i, data.begin() + i + remaining);
            if (remaining < chunk_size) {
                chunk.insert(chunk.end(), chunk_size - remaining, pad_value);
            }
            result.push_back(chunk);
        }
        return result;
    }

    std::vector<std::vector<T>> sliding_window(size_t window_size, size_t step = 1) const {
        validate_size(window_size, "Window size");
        validate_size(step, "Step size");
        std::vector<std::vector<T>> result;
        for (size_t i = 0; i + window_size <= data.size(); i += step) {
            result.push_back(std::vector<T>(data.begin() + i, data.begin() + i + window_size));
        }
        return result;
    }

private:
    void update_chunks() {
        chunks.clear();
        for (size_t i = 0; i < data.size(); i += chunk_size) {
            std::vector<T> chunk;
            for (size_t j = 0; j < chunk_size && i + j < data.size(); ++j) {
                chunk.push_back(data[i + j]);
            }
            chunks.push_back(chunk);
        }
    }
};

#endif // CHUNK_HPP