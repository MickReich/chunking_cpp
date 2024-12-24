#pragma once
#include <stdexcept>
#include <string>

namespace chunk_processing {

/**
 * @brief Base exception class for chunking operations
 */
class ChunkingError : public std::runtime_error {
public:
    explicit ChunkingError(const std::string& message) : std::runtime_error(message) {}
    
    explicit ChunkingError(const char* message) : std::runtime_error(message) {}
};

/**
 * @brief Exception for serialization errors
 */
class SerializationError : public ChunkingError {
public:
    explicit SerializationError(const std::string& message) : ChunkingError(message) {}
};

/**
 * @brief Exception for visualization errors
 */
class VisualizationError : public ChunkingError {
public:
    explicit VisualizationError(const std::string& message) : ChunkingError(message) {}
};

/**
 * @brief Exception for resilience/recovery errors
 */
class ResilienceError : public ChunkingError {
public:
    explicit ResilienceError(const std::string& message) : ChunkingError(message) {}
};

} // namespace chunk_processing 