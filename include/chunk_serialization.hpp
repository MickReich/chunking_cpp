#pragma once
#include <vector>
#include <chrono>
#include <stdexcept>

#ifdef HAVE_JSON
#include <nlohmann/json.hpp>
#endif

namespace chunk_serialization {
template <typename T>
class ChunkSerializer {
public:
    void to_json(const std::vector<std::vector<T>>& chunks) {
        #ifdef HAVE_JSON
        // Implementation using nlohmann::json
        nlohmann::json j;
        j["chunks"] = chunks;
        j["timestamp"] = std::chrono::system_clock::now().time_since_epoch().count();
        return j.dump();
        #else
        throw std::runtime_error("JSON serialization not available");
        #endif
    }

    void to_protobuf(const std::vector<std::vector<T>>& chunks) {
        throw std::runtime_error("Protocol Buffer serialization not implemented");
    }

    void to_msgpack(const std::vector<std::vector<T>>& chunks) {
        throw std::runtime_error("MessagePack serialization not implemented");
    }
};
} // namespace chunk_serialization