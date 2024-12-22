/**
 * @file chunk_strategies.hpp
 * @brief Defines various strategies for chunking data based on different criteria.
 */

#pragma once

#include "chunk.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace chunk_strategies {

/**
 * @brief Base class for chunking strategies
 * @tparam T The type of elements to be chunked
 */
template <typename T>
class ChunkStrategy {
public:
    /**
     * @brief Applies the chunking strategy to the data
     * @param data The data to chunk
     * @return A vector of chunks
     */
    virtual std::vector<std::vector<T>> apply(const std::vector<T>& data) const = 0;
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

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        if (data.empty()) return {};
        
        std::vector<T> sorted_data = data;
        std::sort(sorted_data.begin(), sorted_data.end());
        T threshold = sorted_data[static_cast<size_t>(quantile_ * (data.size() - 1))];
        
        std::vector<std::vector<T>> chunks;
        std::vector<T> current_chunk;
        
        for (const T& value : data) {
            if (!current_chunk.empty() && value > threshold) {
                chunks.push_back(current_chunk);
                current_chunk.clear();
            }
            current_chunk.push_back(value);
        }
        
        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }
        
        return chunks;
    }
};

/**
 * @brief Strategy for chunking based on variance thresholds
 */
template <typename T>
class VarianceStrategy : public ChunkStrategy<T> {
    double threshold_;

    double calculate_variance(const std::vector<T>& chunk) const {
        if (chunk.size() < 2)
            return 0.0;
        double mean = std::accumulate(chunk.begin(), chunk.end(), 0.0) / chunk.size();
        double sq_sum = 0.0;
        for (const T& val : chunk) {
            double diff = val - mean;
            sq_sum += diff * diff;
        }
        return sq_sum / chunk.size();
    }

public:
    explicit VarianceStrategy(double threshold) : threshold_(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        std::vector<std::vector<T>> chunks;
        std::vector<T> current_chunk;

        for (const auto& value : data) {
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

    double calculate_entropy(const std::vector<T>& chunk) const {
        if (chunk.empty())
            return 0.0;

        std::map<T, int> freq;
        for (const auto& val : chunk) {
            freq[val]++;
        }

        double entropy = 0.0;
        for (const auto& [_, count] : freq) {
            double p = static_cast<double>(count) / chunk.size();
            entropy -= p * std::log2(p);
        }

        return entropy;
    }

public:
    explicit EntropyStrategy(double threshold) : threshold_(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        std::vector<std::vector<T>> chunks;
        std::vector<T> current_chunk;

        for (const auto& value : data) {
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

template <typename T>
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

template <typename T>
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

template <typename T>
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

template <typename T>
class DynamicThresholdStrategy {
public:
    DynamicThresholdStrategy(T initial_threshold, T min_threshold, double decay_rate)
        : initial_threshold_(initial_threshold), min_threshold_(min_threshold),
          decay_rate_(decay_rate) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) {
        Chunk<T> chunker(1);
        chunker.add(data);

        T current_threshold = initial_threshold_;
        return chunker.chunk_by_predicate([this, &current_threshold](T x) {
            static T last = x;
            bool start_new = std::abs(x - last) > current_threshold;
            current_threshold =
                std::max(min_threshold_, static_cast<T>(current_threshold * decay_rate_));
            last = x;
            return start_new;
        });
    }

private:
    T initial_threshold_;
    T min_threshold_;
    double decay_rate_;
};

template <>
class EntropyStrategy<std::string> : public ChunkStrategy<std::string> {
    double threshold_;

    double calculate_entropy(const std::vector<std::string>& values) const {
        if (values.empty())
            return 0.0;

        std::map<std::string, int> freq;
        for (const auto& val : values) {
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

    std::vector<std::vector<std::string>> apply(const std::vector<std::string>& data) const override {
        std::vector<std::vector<std::string>> chunks;
        std::vector<std::string> current_chunk;

        for (const auto& value : data) {
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

} // namespace chunk_strategies