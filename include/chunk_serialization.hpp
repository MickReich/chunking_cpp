// Add support for various serialization formats
namespace chunk_serialization {
template <typename T>
class ChunkSerializer {
public:
    void to_json(const std::vector<std::vector<T>>& chunks);
    void to_protobuf(const std::vector<std::vector<T>>& chunks);
    void to_msgpack(const std::vector<std::vector<T>>& chunks);
};
} // namespace chunk_serialization