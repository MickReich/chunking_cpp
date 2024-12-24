template <typename T>
class CHUNK_EXPORT WaveletChunking {
public:
    WaveletChunking(size_t window_size, double threshold)
        : window_size_(window_size), threshold_(threshold) {
        if (window_size == 0) {
            throw std::invalid_argument("Window size cannot be zero");
        }
    }

    void set_window_size(size_t new_size) {
        if (new_size == 0) {
            throw std::invalid_argument("Window size cannot be zero");
        }
        window_size_ = new_size;
    }

    void set_threshold(double new_threshold) {
        threshold_ = new_threshold;
    }

    std::vector<std::vector<T>> chunk(const std::vector<T>& data) {
        if (data.empty()) {
            throw std::invalid_argument("Cannot chunk empty data");
        }

        std::vector<std::vector<T>> result;
        std::vector<T> current_chunk;

        for (size_t i = 0; i < data.size(); ++i) {
            current_chunk.push_back(data[i]);

            if (current_chunk.size() >= window_size_) {
                // Perform wavelet analysis to detect boundaries
                if (detect_boundary(current_chunk)) {
                    result.push_back(current_chunk);
                    current_chunk.clear();
                }
            }
        }

        if (!current_chunk.empty()) {
            result.push_back(current_chunk);
        }

        return result;
    }

private:
    size_t window_size_;
    double threshold_;

    bool detect_boundary(const std::vector<T>& window) {
        if (window.size() < 2)
            return false;

        // Simple boundary detection based on value differences
        double max_diff = 0.0;
        for (size_t i = 1; i < window.size(); ++i) {
            max_diff = std::max(max_diff, std::abs(static_cast<double>(window[i] - window[i - 1])));
        }
        return max_diff > threshold_;
    }
};