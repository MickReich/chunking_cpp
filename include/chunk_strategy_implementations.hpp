#pragma once

#include "chunk_strategies.hpp"
#include <memory>
#include <vector>

namespace chunk_processing {

template <typename T>
class NeuralChunkingStrategy : public ChunkStrategy<T> {
private:
    double threshold_;

public:
    explicit NeuralChunkingStrategy() : threshold_(0.5) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;

        std::vector<T> current_chunk;
        for (const auto& value : data) {
            if (!current_chunk.empty() && std::abs(value - current_chunk.back()) > threshold_) {
                result.push_back(current_chunk);
                current_chunk.clear();
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
class SimilarityChunkingStrategy : public ChunkStrategy<T> {
private:
    double similarity_threshold_;

public:
    explicit SimilarityChunkingStrategy(double threshold) : similarity_threshold_(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        std::vector<std::vector<T>> result;
        if (data.empty())
            return result;

        std::vector<T> current_chunk;
        for (const auto& value : data) {
            if (!current_chunk.empty() &&
                calculate_similarity(value, current_chunk.back()) < similarity_threshold_) {
                result.push_back(current_chunk);
                current_chunk.clear();
            }
            current_chunk.push_back(value);
        }
        if (!current_chunk.empty()) {
            result.push_back(current_chunk);
        }
        return result;
    }

private:
    double calculate_similarity(const T& a, const T& b) const {
        return 1.0 / (1.0 + std::abs(static_cast<double>(a) - static_cast<double>(b)));
    }
};

} // namespace chunk_processing