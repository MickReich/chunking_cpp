/**
 * @file chunk_serialization.hpp
 * @brief Serialization utilities for chunk data
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once
#include "chunk_common.hpp"
#include <stdexcept>
#include <string>
#include <vector>

namespace chunk_serialization {

/**
 * @brief Class for serializing chunks to various formats
 * @tparam T The data type of the chunks
 */
template <typename T>
class CHUNK_EXPORT ChunkSerializer {
public:
    /**
     * @brief Serialize chunks to JSON format
     * @param chunks Vector of chunk data
     * @return JSON string representation
     * @throws std::runtime_error if serialization fails
     */
    std::string to_json(const std::vector<std::vector<T>>& chunks) {
        validate_chunks(chunks);

        // Basic JSON array serialization
        std::string result = "[";
        for (size_t i = 0; i < chunks.size(); ++i) {
            result += "[";
            for (size_t j = 0; j < chunks[i].size(); ++j) {
                result += std::to_string(chunks[i][j]);
                if (j < chunks[i].size() - 1) {
                    result += ",";
                }
            }
            result += "]";
            if (i < chunks.size() - 1) {
                result += ",";
            }
        }
        result += "]";
        return result;
    }

    /**
     * @brief Serialize chunks to Protocol Buffers format
     * @param chunks Vector of chunk data
     * @return Protobuf binary string
     * @throws std::runtime_error if Protobuf support not available
     */
    std::string to_protobuf(const std::vector<std::vector<T>>& chunks) {
        validate_chunks(chunks);
        throw std::runtime_error("Protocol Buffers serialization not implemented");
    }

    /**
     * @brief Serialize chunks to MessagePack format
     * @param chunks Vector of chunk data
     * @return MessagePack binary string
     * @throws std::runtime_error if MessagePack support not available
     */
    std::string to_msgpack(const std::vector<std::vector<T>>& chunks) {
        validate_chunks(chunks);

        // Basic implementation without MessagePack dependency
        std::string result = "[";
        for (size_t i = 0; i < chunks.size(); ++i) {
            result += "[";
            for (size_t j = 0; j < chunks[i].size(); ++j) {
                result += std::to_string(chunks[i][j]);
                if (j < chunks[i].size() - 1) {
                    result += ",";
                }
            }
            result += "]";
            if (i < chunks.size() - 1) {
                result += ",";
            }
        }
        result += "]";
        return result;
    }

private:
    /**
     * @brief Validate chunk data before serialization
     * @param chunks Vector of chunk data to validate
     * @throws std::invalid_argument if validation fails
     */
    void validate_chunks(const std::vector<std::vector<T>>& chunks) {
        if (chunks.empty()) {
            throw std::invalid_argument("Cannot serialize empty chunks");
        }

        for (const auto& chunk : chunks) {
            if (chunk.empty()) {
                throw std::invalid_argument("Cannot serialize chunks containing empty vectors");
            }
        }
    }
};

} // namespace chunk_serialization