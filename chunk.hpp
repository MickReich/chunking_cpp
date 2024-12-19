template <typename T>
class Chunk {
private:
    std::vector<T> data_;
    size_t chunk_size_;

public:
    // Constructor with chunk size
    explicit Chunk(size_t chunk_size) : chunk_size_(chunk_size) {
        if (chunk_size == 0) {
            throw std::invalid_argument("Chunk size must be greater than 0");
        }
    }

    // Add data to the chunk
    void add(const T& value) {
        data_.push_back(value);
    }

    // Add a vector of data
    void add(const std::vector<T>& values) {
        data_.insert(data_.end(), values.begin(), values.end());
    }

    // Get chunks as vector of vectors
    std::vector<std::vector<T>> get_chunks() const {
        std::vector<std::vector<T>> chunks;
        
        for (size_t i = 0; i < data_.size(); i += chunk_size_) {
            std::vector<T> chunk;
            size_t end = std::min(i + chunk_size_, data_.size());
            chunk.insert(chunk.end(), data_.begin() + i, data_.begin() + end);
            chunks.push_back(chunk);
        }
        
        return chunks;
    }

    // Get a specific chunk by index
    std::vector<T> get_chunk(size_t index) const {
        size_t start = index * chunk_size_;
        if (start >= data_.size()) {
            throw std::out_of_range("Chunk index out of range");
        }
        
        size_t end = std::min(start + chunk_size_, data_.size());
        return std::vector<T>(data_.begin() + start, data_.begin() + end);
    }

    // Get the number of chunks
    size_t chunk_count() const {
        return (data_.size() + chunk_size_ - 1) / chunk_size_;
    }

    // Get the chunk size
    size_t chunk_size() const {
        return chunk_size_;
    }

    // Get total size of data
    size_t size() const {
        return data_.size();
    }

    // Clear all data
    void clear() {
        data_.clear();
    }

    // Check if empty
    bool empty() const {
        return data_.empty();
    }
};
