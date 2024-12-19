#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef>

namespace chunk_config {

struct ChunkSettings {
    static constexpr size_t DEFAULT_CHUNK_SIZE = 4;
    static constexpr size_t MAX_CHUNK_SIZE = 1024;
    static constexpr size_t MIN_CHUNK_SIZE = 1;
};

struct BufferSettings {
    static constexpr size_t DEFAULT_BUFFER_SIZE = 1024;
    static constexpr size_t MAX_BUFFER_SIZE = 1024 * 1024;
    static constexpr size_t MIN_BUFFER_SIZE = 1;
};

} // namespace chunk_config

#endif // CONFIG_HPP