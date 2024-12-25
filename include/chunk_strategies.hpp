/**
 * @file chunk_strategies.hpp
 * @brief Defines various strategies for chunking data based on different criteria.
 */

#pragma once

#include "chunk_common.hpp"
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <vector>

// Forward declarations
namespace chunk_processing {
template <typename T>
class ChunkStrategy;
template <typename T>
class VarianceStrategy;
template <typename T>
class EntropyStrategy;
} // namespace chunk_processing

// Include implementation files
#include "sub_chunk_strategies.hpp"

namespace chunk_processing {

template <typename T>
class ChunkStrategy {
public:
    virtual ~ChunkStrategy() = default;
    virtual std::vector<std::vector<T>> apply(const std::vector<T>& data) const = 0;
};

template <typename T>
class PatternBasedStrategy : public ChunkStrategy<T> {
private:
    std::function<bool(T)> predicate_;
    size_t pattern_size_;

public:
    // Constructor for predicate-based chunking
    explicit PatternBasedStrategy(std::function<bool(T)> predicate)
        : predicate_(std::move(predicate)), pattern_size_(0) {}

    // Constructor for size-based pattern chunking
    explicit PatternBasedStrategy(size_t pattern_size) : pattern_size_(pattern_size) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;

        if (pattern_size_ > 0) {
            // Size-based pattern chunking
            for (size_t i = 0; i < data.size(); i += pattern_size_) {
                size_t end = std::min(i + pattern_size_, data.size());
                result.emplace_back(data.begin() + i, data.begin() + end);
            }
        } else {
            // Predicate-based chunking
            std::vector<T> current_chunk;
            for (const auto& value : data) {
                if (!current_chunk.empty() && predicate_(value)) {
                    result.push_back(current_chunk);
                    current_chunk.clear();
                }
                current_chunk.push_back(value);
            }
            if (!current_chunk.empty()) {
                result.push_back(current_chunk);
            }
        }
        return result;
    }
};

template <typename T>
class VarianceStrategy : public ChunkStrategy<T> {
private:
    double threshold_;

public:
    explicit VarianceStrategy(double threshold) : threshold_(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;

        std::vector<T> current_chunk;
        double sum = 0.0;
        double sum_sq = 0.0;
        size_t count = 0;

        for (const auto& value : data) {
            if (!current_chunk.empty()) {
                sum += static_cast<double>(value);
                sum_sq += static_cast<double>(value) * static_cast<double>(value);
                count++;

                double mean = sum / count;
                double variance = (sum_sq / count) - (mean * mean);

                if (variance > threshold_) {
                    result.push_back(current_chunk);
                    current_chunk.clear();
                    sum = 0.0;
                    sum_sq = 0.0;
                    count = 0;
                }
            }
            current_chunk.push_back(value);
        }

        if (!current_chunk.empty()) {
            result.push_back(current_chunk);
        }

        return result;
    }
};

template <typename T>
class EntropyStrategy : public ChunkStrategy<T> {
private:
    double threshold_;

    double calculate_entropy(const std::vector<T>& chunk) const {
        if (chunk.empty())
            return 0.0;

        // Calculate frequency distribution
        std::map<T, double> freq;
        for (const auto& val : chunk) {
            freq[val] += 1.0;
        }

        // Calculate entropy
        double entropy = 0.0;
        double n = static_cast<double>(chunk.size());
        for (const auto& pair : freq) {
            double p = pair.second / n;
            entropy -= p * std::log2(p);
        }

        return entropy;
    }

public:
    explicit EntropyStrategy(double threshold) : threshold_(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;

        std::vector<T> current_chunk;
        for (const auto& value : data) {
            current_chunk.push_back(value);

            if (current_chunk.size() > 1) {
                double entropy = calculate_entropy(current_chunk);
                if (entropy > threshold_) {
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
};

} // namespace chunk_processing