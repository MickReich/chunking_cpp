#pragma once

#include <algorithm>
#include <cmath>
#include <vector>
#include <utility>
#include <stdexcept>

namespace chunk_compression {

/**
 * @brief Compression strategies for chunk data
 */
template <typename T>
class ChunkCompressor {
public:
    /**
     * @brief Run-length encode a chunk
     * @param chunk Input chunk
     * @return Encoded chunk with value-count pairs
     */
    static std::vector<std::pair<T, size_t>> run_length_encode(const std::vector<T>& chunk) {
        std::vector<std::pair<T, size_t>> result;
        if (chunk.empty()) {
            return result;
        }

        result.reserve(chunk.size() / 2 + 1);  // Reasonable initial capacity
        
        T current = chunk[0];
        size_t count = 1;

        for (size_t i = 1; i < chunk.size(); ++i) {
            if (chunk[i] == current) {
                ++count;
            } else {
                result.emplace_back(current, count);
                current = chunk[i];
                count = 1;
            }
        }
        
        // Don't forget the last group
        if (count > 0) {
            result.emplace_back(current, count);
        }

        return result;
    }

    /**
     * @brief Delta encode a chunk
     * @param chunk Input chunk
     * @return Delta-encoded chunk
     */
    static std::vector<T> delta_encode(const std::vector<T>& chunk) {
        if (chunk.empty()) {
            return {};
        }

        std::vector<T> result;
        result.reserve(chunk.size());  // Pre-allocate space
        result.push_back(chunk[0]);    // First value stays the same

        for (size_t i = 1; i < chunk.size(); ++i) {
            // Check for potential overflow/underflow
            try {
                result.push_back(chunk[i] - chunk[i - 1]);
            } catch (const std::exception&) {
                // If arithmetic operation fails, return empty result
                return {};
            }
        }

        return result;
    }

    /**
     * @brief Delta decode a chunk
     * @param chunk Delta-encoded chunk
     * @return Decoded chunk
     */
    static std::vector<T> delta_decode(const std::vector<T>& chunk) {
        if (chunk.empty()) {
            return {};
        }

        std::vector<T> result;
        result.reserve(chunk.size());  // Pre-allocate space
        result.push_back(chunk[0]);    // First value stays the same

        for (size_t i = 1; i < chunk.size(); ++i) {
            try {
                result.push_back(result.back() + chunk[i]);
            } catch (const std::exception&) {
                // If arithmetic operation fails, return empty result
                return {};
            }
        }

        return result;
    }

    /**
     * @brief Calculate compression ratio
     * @param original Original data
     * @param compressed Compressed data
     * @return Compression ratio (original size / compressed size), or 0.0 if invalid
     */
    static double calculate_compression_ratio(const std::vector<T>& original,
                                           const std::vector<std::pair<T, size_t>>& compressed) {
        // Handle edge cases
        if (original.empty() || compressed.empty()) {
            return 0.0;
        }

        try {
            // Calculate sizes safely
            size_t original_size = original.size();
            if (original_size == 0) return 0.0;

            // Verify compressed data integrity
            size_t total_compressed_count = 0;
            for (const auto& pair : compressed) {
                // Check for overflow before adding
                if (pair.second > std::numeric_limits<size_t>::max() - total_compressed_count) {
                    return 0.0;  // Overflow would occur
                }
                total_compressed_count += pair.second;
            }

            // Verify that compressed data represents the same amount of elements
            if (total_compressed_count != original_size) {
                return 0.0;  // Data mismatch
            }

            // Calculate actual sizes in bytes
            size_t original_bytes = original_size * sizeof(T);
            size_t compressed_bytes = compressed.size() * (sizeof(T) + sizeof(size_t));
            
            // Prevent division by zero
            if (compressed_bytes == 0) {
                return 0.0;
            }

            // Calculate ratio safely
            return static_cast<double>(original_bytes) / compressed_bytes;

        } catch (const std::exception&) {
            // Handle any unexpected errors safely
            return 0.0;
        }
    }
};

} // namespace chunk_compression