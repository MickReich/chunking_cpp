#ifndef CHUNK_COMPRESSION_HPP
#define CHUNK_COMPRESSION_HPP

#include <algorithm>
#include <cmath>
#include <vector>

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
        if (chunk.empty())
            return result;

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
        result.emplace_back(current, count);

        return result;
    }

    /**
     * @brief Delta encode a chunk
     * @param chunk Input chunk
     * @return Delta-encoded chunk
     */
    static std::vector<T> delta_encode(const std::vector<T>& chunk) {
        if (chunk.empty())
            return {};

        std::vector<T> result;
        result.reserve(chunk.size());
        result.push_back(chunk[0]);

        for (size_t i = 1; i < chunk.size(); ++i) {
            result.push_back(chunk[i] - chunk[i - 1]);
        }

        return result;
    }

    /**
     * @brief Delta decode a chunk
     * @param chunk Delta-encoded chunk
     * @return Decoded chunk
     */
    static std::vector<T> delta_decode(const std::vector<T>& chunk) {
        if (chunk.empty())
            return {};

        std::vector<T> result;
        result.reserve(chunk.size());
        result.push_back(chunk[0]);

        for (size_t i = 1; i < chunk.size(); ++i) {
            result.push_back(result.back() + chunk[i]);
        }

        return result;
    }
};

} // namespace chunk_compression

#endif // CHUNK_COMPRESSION_HPP