/**
 * @file chunk_serialization.hpp
 * @brief Serialization utilities for chunk data
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include "chunk_common.hpp"

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
    std::string to_json(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Serialize chunks to Protocol Buffers format
     * @param chunks Vector of chunk data
     * @return Protobuf binary string
     * @throws std::runtime_error if Protobuf support not available
     */
    std::string to_protobuf(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Serialize chunks to MessagePack format
     * @param chunks Vector of chunk data
     * @return MessagePack binary string
     * @throws std::runtime_error if MessagePack support not available
     */
    std::string to_msgpack(const std::vector<std::vector<T>>& chunks);

private:
    /**
     * @brief Validate chunk data before serialization
     * @param chunks Vector of chunk data to validate
     * @throws std::invalid_argument if validation fails
     */
    void validate_chunks(const std::vector<std::vector<T>>& chunks);
};

} // namespace chunk_serialization