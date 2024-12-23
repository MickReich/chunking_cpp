template<typename T>
class MultiCriteriaStrategy {
private:
    mutable std::mutex strategy_mutex;
    size_t min_size;
    double similarity_threshold;

public:
    MultiCriteriaStrategy(size_t min_size_, double similarity_threshold_)
        : min_size(min_size_), similarity_threshold(similarity_threshold_) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const {
        std::lock_guard<std::mutex> lock(strategy_mutex);
        try {
            if (data.empty()) return {};

            std::vector<std::vector<T>> result;
            std::vector<T> current_chunk;
            
            for (size_t i = 0; i < data.size(); ++i) {
                current_chunk.push_back(data[i]);
                
                // Check both criteria
                bool size_criterion = current_chunk.size() >= min_size;
                bool similarity_criterion = i > 0 && 
                    std::abs(data[i] - data[i-1]) > similarity_threshold;
                
                if ((size_criterion || similarity_criterion) && !current_chunk.empty()) {
                    result.push_back(current_chunk);
                    current_chunk.clear();
                }
            }
            
            if (!current_chunk.empty()) {
                result.push_back(current_chunk);
            }
            
            return result;
        } catch (const std::exception& e) {
            throw std::runtime_error("Error in MultiCriteriaStrategy: " + std::string(e.what()));
        }
    }

    // Helper function to validate inputs
    void validate_inputs(const std::vector<T>& data) const {
        if (min_size == 0) {
            throw std::invalid_argument("Minimum size must be positive");
        }
        if (similarity_threshold < 0) {
            throw std::invalid_argument("Similarity threshold must be non-negative");
        }
        if (data.size() < min_size) {
            throw std::invalid_argument("Input data size must be at least minimum chunk size");
        }
    }
}; 