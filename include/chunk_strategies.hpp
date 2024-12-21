#pragma once

#include "chunk.hpp"
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <stdexcept>
#include <vector>
#include <functional>

namespace chunk_strategies {

/**
 * @brief Base class for chunk strategies
 * @tparam T The type of elements to process
 */
template <typename T>
class ChunkStrategy {
public:
    virtual std::vector<std::vector<T>> apply(const std::vector<T>& data) = 0;
    virtual ~ChunkStrategy() = default;
};

/**
 * @brief Strategy for chunking based on quantile thresholds
 */
template <typename T>
class QuantileStrategy : public ChunkStrategy<T> {
    double quantile_;

public:
    explicit QuantileStrategy(double quantile) : quantile_(quantile) {
        if (quantile < 0.0 || quantile > 1.0) {
            throw std::invalid_argument("Quantile must be between 0 and 1");
        }
    }

    std::vector<std::vector<T>> apply(const std::vector<T>& data) override {
        if (data.empty())
            return {};
        if (data.size() == 1)
            return {data};

        std::vector<T> sorted = data;
        std::sort(sorted.begin(), sorted.end());
        T threshold = sorted[sorted.size() / 2];

        std::vector<std::vector<T>> chunks;
        std::vector<T> lower_chunk, upper_chunk;

        for (const T& value : data) {
            if (value <= threshold) {
                lower_chunk.push_back(value);
            } else {
                upper_chunk.push_back(value);
            }
        }

        chunks.push_back(lower_chunk);
        chunks.push_back(upper_chunk);

        return chunks;
    }
};

/**
 * @brief Strategy for chunking based on variance thresholds
 */
template <typename T>
class VarianceStrategy : public ChunkStrategy<T> {
    double threshold_;

    double calculate_variance(const std::vector<T>& values) {
        if (values.size() < 2)
            return 0.0;
        double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
        double sq_sum = 0.0;
        for (const T& val : values) {
            sq_sum += (val - mean) * (val - mean);
        }
        return sq_sum / values.size();
    }

public:
    explicit VarianceStrategy(double threshold) : threshold_(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) override {
        std::vector<std::vector<T>> chunks;
        std::vector<T> current_chunk;

        for (const T& value : data) {
            current_chunk.push_back(value);
            if (calculate_variance(current_chunk) > threshold_) {
                if (current_chunk.size() > 1) {
                    current_chunk.pop_back();
                    chunks.push_back(current_chunk);
                    current_chunk = {value};
                }
            }
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }
};

/**
 * @brief Strategy for chunking based on entropy thresholds
 */
template <typename T>
class EntropyStrategy : public ChunkStrategy<T> {
    double threshold_;

    double calculate_entropy(const std::vector<T>& values) {
        if (values.empty())
            return 0.0;

        std::map<T, int> freq;
        for (const T& val : values) {
            freq[val]++;
        }

        double entropy = 0.0;
        for (const auto& [_, count] : freq) {
            double p = static_cast<double>(count) / values.size();
            entropy -= p * std::log2(p);
        }

        return entropy;
    }

public:
    explicit EntropyStrategy(double threshold) : threshold_(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) override {
        std::vector<std::vector<T>> chunks;
        std::vector<T> current_chunk;

        for (const T& value : data) {
            current_chunk.push_back(value);
            if (calculate_entropy(current_chunk) > threshold_) {
                if (current_chunk.size() > 1) {
                    current_chunk.pop_back();
                    chunks.push_back(current_chunk);
                    current_chunk = {value};
                }
            }
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }
};

template<typename T>
class PatternBasedStrategy {
public:
    explicit PatternBasedStrategy(std::function<bool(T)> pattern_detector)
        : pattern_detector_(pattern_detector) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) {
        Chunk<T> chunker(1);
        chunker.add(data);
        return chunker.chunk_by_predicate(pattern_detector_);
    }

private:
    std::function<bool(T)> pattern_detector_;
};

template<typename T>
class AdaptiveStrategy {
public:
    explicit AdaptiveStrategy(T initial_threshold, 
                            std::function<T(const std::vector<T>&)> metric_calculator)
        : threshold_(initial_threshold), metric_calculator_(metric_calculator) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) {
        Chunk<T> chunker(1);
        chunker.add(data);
        return chunker.chunk_by_statistic(threshold_, metric_calculator_);
    }

private:
    T threshold_;
    std::function<T(const std::vector<T>&)> metric_calculator_;
};

template<typename T>
class MultiCriteriaStrategy {
public:
    MultiCriteriaStrategy(T similarity_threshold, size_t size_threshold)
        : similarity_threshold_(similarity_threshold), size_threshold_(size_threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) {
        // Step 1: Similarity-based chunking
        Chunk<T> initial_chunker(1);
        initial_chunker.add(data);
        auto similarity_chunks = initial_chunker.chunk_by_similarity(similarity_threshold_);

        // Step 2: Size-based chunking
        std::vector<std::vector<T>> result;
        for (const auto& chunk : similarity_chunks) {
            Chunk<T> size_chunker(size_threshold_);
            size_chunker.add(chunk);
            auto sized_chunks = size_chunker.get_chunks();
            result.insert(result.end(), sized_chunks.begin(), sized_chunks.end());
        }
        return result;
    }

private:
    T similarity_threshold_;
    size_t size_threshold_;
};

template<typename T>
class DynamicThresholdStrategy {
public:
    DynamicThresholdStrategy(T initial_threshold, T min_threshold, double decay_rate)
        : initial_threshold_(initial_threshold), 
          min_threshold_(min_threshold),
          decay_rate_(decay_rate) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) {
        Chunk<T> chunker(1);
        chunker.add(data);
        
        T current_threshold = initial_threshold_;
        return chunker.chunk_by_predicate([this, &current_threshold](T x) {
            static T last = x;
            bool start_new = std::abs(x - last) > current_threshold;
            current_threshold = std::max(min_threshold_, 
                                       static_cast<T>(current_threshold * decay_rate_));
            last = x;
            return start_new;
        });
    }

private:
    T initial_threshold_;
    T min_threshold_;
    double decay_rate_;
};

} // namespace chunk_strategies