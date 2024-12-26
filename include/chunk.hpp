#pragma once

#include "chunk_common.hpp"
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace chunk_processing {

/**
 * @brief A template class for managing and processing data in chunks
 * @tparam T The type of elements stored in the chunk
 */
template <typename T>
class CHUNK_EXPORT Chunk {
private:
    size_t chunk_size_;
    std::vector<T> data_;
    std::vector<std::vector<T>> chunks_;

    // Helper class for handling jagged arrays
    template <typename U>
    class JaggedVectorHandler {
    public:
        static std::vector<std::vector<U>> normalize(const std::vector<std::vector<U>>& jagged) {
            if (jagged.empty())
                return {};

            // Find maximum size
            size_t max_size = 0;
            for (const auto& row : jagged) {
                max_size = std::max(max_size, row.size());
            }

            // Normalize by padding with default values
            std::vector<std::vector<U>> normalized;
            normalized.reserve(jagged.size());

            for (const auto& row : jagged) {
                std::vector<U> normalized_row = row;
                normalized_row.resize(max_size, U{}); // Pad with default values
                normalized.push_back(std::move(normalized_row));
            }

            return normalized;
        }

        static std::vector<std::vector<std::vector<U>>>
        normalize_3d(const std::vector<std::vector<std::vector<U>>>& jagged_3d) {
            if (jagged_3d.empty())
                return {};

            // Find maximum sizes
            size_t max_rows = 0, max_cols = 0;
            for (const auto& matrix : jagged_3d) {
                max_rows = std::max(max_rows, matrix.size());
                for (const auto& row : matrix) {
                    max_cols = std::max(max_cols, row.size());
                }
            }

            // Normalize by padding
            std::vector<std::vector<std::vector<U>>> normalized;
            normalized.reserve(jagged_3d.size());

            for (const auto& matrix : jagged_3d) {
                std::vector<std::vector<U>> norm_matrix;
                norm_matrix.reserve(max_rows);

                for (const auto& row : matrix) {
                    std::vector<U> norm_row = row;
                    norm_row.resize(max_cols, U{});
                    norm_matrix.push_back(std::move(norm_row));
                }

                while (norm_matrix.size() < max_rows) {
                    norm_matrix.push_back(std::vector<U>(max_cols));
                }

                normalized.push_back(std::move(norm_matrix));
            }

            return normalized;
        }
    };

    // Helper function for validation
    void validate_size(size_t size, const std::string& param) const {
        if (size == 0) {
            throw std::invalid_argument(param + " must be greater than 0");
        }
    }
    // Ensure data is properly copied
    std::vector<std::vector<T>> make_chunks(size_t size) const {
        std::vector<std::vector<T>> result;
        result.reserve((data_.size() + size - 1) / size);

        for (size_t i = 0; i < data_.size(); i += size) {
            size_t chunk_end = std::min(i + size, data_.size());
            result.emplace_back(data_.begin() + i, data_.begin() + chunk_end);
        }
        return result;
    }

    void update_chunks() {
        chunks_.clear();
        for (size_t i = 0; i < data_.size(); i += chunk_size_) {
            std::vector<T> chunk;
            for (size_t j = 0; j < chunk_size_ && i + j < data_.size(); ++j) {
                chunk.push_back(data_[i + j]);
            }
            chunks_.push_back(chunk);
        }
    }

    // Add support for checking dimensionality
    template <typename U>
    static constexpr size_t get_depth() {
        if constexpr (is_vector<U>::value)
            return 1 + get_depth<typename U::value_type>();
        return 0;
    }

public:
    explicit Chunk(size_t chunk_size = 1) : chunk_size_(chunk_size) {
        validate_size(chunk_size, "Chunk size");
    }

    void add(const T& element) {
        data_.push_back(element);
        update_chunks();
    }

    void add(const std::vector<T>& elements) {
        data_.insert(data_.end(), elements.begin(), elements.end());
        update_chunks();
    }

    std::vector<std::vector<T>> chunk_by_size(size_t size) {
        if (data_.empty()) {
            throw std::invalid_argument("Cannot chunk empty data");
        }
        if (size == 0) {
            throw std::invalid_argument("Chunk size cannot be zero");
        }
        return make_chunks(size);
    }

    std::vector<std::vector<T>> chunk_by_threshold(T threshold) {
        if (data_.empty()) {
            throw std::invalid_argument("Cannot chunk empty data");
        }
        if (data_.size() < chunk_size_) {
            throw std::invalid_argument("Input size must be at least chunk size");
        }
        if (threshold <= T{}) {
            throw std::invalid_argument("Threshold must be positive");
        }

        std::vector<std::vector<T>> result;
        std::vector<T> current_chunk;
        T running_sum = T{};

        for (const T& value : data_) {
            if (running_sum + value > threshold && !current_chunk.empty()) {
                result.push_back(current_chunk);
                current_chunk.clear();
                running_sum = T{};
            }
            current_chunk.push_back(value);
            running_sum += value;
        }

        if (!current_chunk.empty()) {
            result.push_back(current_chunk);
        }

        return result;
    }

    std::vector<std::vector<T>> get_chunks() const {
        return chunks_;
    }

    size_t size() const {
        return data_.size();
    }

    size_t chunk_count() const {
        return (data_.size() + chunk_size_ - 1) / chunk_size_;
    }

    size_t get_chunk_size() const {
        return chunk_size_;
    }
    const std::vector<T>& get_data() const {
        return data_;
    }

    void set_chunk_size(size_t new_size) {
        validate_size(new_size, "Chunk size");
        chunk_size_ = new_size;
        update_chunks();
    }

    // Add methods to handle multi-dimensional data
    template <typename U = T>
    std::enable_if_t<is_vector<U>::value> add(const U& nested_data) {
        // First validate dimensions
        if (chunk_processing::is_jagged(nested_data)) {
            throw std::invalid_argument("Jagged arrays are not supported");
        }

        // For 3D arrays, check additional level
        if constexpr (chunk_processing::is_vector<typename U::value_type::value_type>::value) {
            if (chunk_processing::is_jagged_3d(
                    reinterpret_cast<const std::vector<
                        std::vector<std::vector<typename U::value_type::value_type>>>&>(
                        nested_data))) {
                throw std::invalid_argument("Jagged 3D arrays are not supported");
            }
        }

        data_.push_back(nested_data);
        update_chunks();
    }

    // Get the dimensionality of the data
    static constexpr size_t dimensions() {
        return get_depth<T>();
    }

    // Add methods to handle jagged arrays
    template <typename U>
    std::vector<std::vector<U>> handle_jagged_2d(const std::vector<std::vector<U>>& data) {
        if (!chunk_processing::is_jagged(data)) {
            return data; // Already uniform
        }
        return JaggedVectorHandler<U>::normalize(data);
    }

    template <typename U>
    std::vector<std::vector<std::vector<U>>>
    handle_jagged_3d(const std::vector<std::vector<std::vector<U>>>& data) {
        if (!chunk_processing::is_jagged_3d(data)) {
            return data; // Already uniform
        }
        return JaggedVectorHandler<U>::normalize_3d(data);
    }

    template <typename U>
    void validate_dimensions(const std::vector<U>& data, size_t expected_size = 0) {
        if constexpr (chunk_processing::is_vector<U>::value) {
            // For jagged arrays, normalize instead of throwing error
            if (chunk_processing::is_jagged(data)) {
                auto normalized = handle_jagged_2d(data);
                if (expected_size > 0 && normalized.size() != expected_size) {
                    throw std::invalid_argument("Inconsistent dimensions after normalization");
                }
                return;
            }

            // For 3D arrays, handle jagged data similarly
            if constexpr (chunk_processing::is_vector<typename U::value_type>::value) {
                if (chunk_processing::is_jagged_3d(
                        reinterpret_cast<const std::vector<
                            std::vector<std::vector<typename U::value_type::value_type>>>&>(
                            data))) {
                    auto normalized = handle_jagged_3d(data);
                    if (expected_size > 0 && normalized.size() != expected_size) {
                        throw std::invalid_argument("Inconsistent dimensions after 3D normalization");
                    }
                    return;
                }
            }

            // Check size consistency for non-jagged arrays
            if (expected_size > 0 && data.size() != expected_size) {
                throw std::invalid_argument("Inconsistent dimensions in nested array");
            }

            // Recursively validate inner dimensions
            if (!data.empty()) {
                validate_dimensions(data[0], data[0].size());
            }
        }
    }
};

} // namespace chunk_processing
