/**
 * @file sub_chunk_strategies.hpp
 * @brief Advanced sub-chunking strategies for hierarchical data processing
 *
 * This file provides implementations of various sub-chunking strategies:
 * - Recursive sub-chunking for depth-based processing
 * - Hierarchical sub-chunking for level-based processing
 * - Conditional sub-chunking for property-based processing
 */
#pragma once

#include "chunk_strategies.hpp"
#include <functional>
#include <memory>
#include <stdexcept>
#include <vector>

namespace chunk_processing {

template <typename T>
class RecursiveSubChunkStrategy : public ChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> base_strategy_;
    size_t max_depth_;
    size_t min_size_;

    std::vector<std::vector<T>> recursive_apply(const std::vector<T>& data, size_t depth) const {
        // Validate input
        if (data.empty()) {
            return {};
        }

        // Check termination conditions
        if (depth >= max_depth_ || data.size() <= min_size_) {
            return {data};
        }

        try {
            // Apply base strategy and validate its output
            auto chunks = base_strategy_->apply(data);
            
            // Protect against invalid results from base strategy
            if (chunks.empty()) {
                return {data};
            }

            // NEW: Check if any resulting chunk is smaller than min_size
            bool has_small_chunks = false;
            for (const auto& chunk : chunks) {
                if (chunk.size() < min_size_) {
                    has_small_chunks = true;
                    break;
                }
            }
            
            // If any chunk is too small, return original data as single chunk
            if (has_small_chunks) {
                return {data};
            }

            // Verify the chunks actually split the data
            size_t total_elements = 0;
            for (const auto& chunk : chunks) {
                total_elements += chunk.size();
            }
            
            // If chunks don't preserve data or didn't actually split, return original
            if (total_elements != data.size() || chunks.size() <= 1) {
                return {data};
            }

            // Process sub-chunks
            std::vector<std::vector<T>> result;
            result.reserve(chunks.size() * 2);  // Reasonable initial capacity

            for (const auto& chunk : chunks) {
                if (chunk.empty()) {
                    continue;  // Skip empty chunks
                }
                
                auto sub_chunks = recursive_apply(chunk, depth + 1);
                result.insert(result.end(), sub_chunks.begin(), sub_chunks.end());
            }

            return result.empty() ? std::vector<std::vector<T>>{{data}} : result;

        } catch (const std::exception&) {
            // If any exception occurs during processing, return original data as single chunk
            return {data};
        }
    }

public:
    RecursiveSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy, 
                             size_t max_depth,
                             size_t min_size)
        : base_strategy_(strategy), max_depth_(max_depth), min_size_(min_size) {
        if (!strategy) {
            throw std::invalid_argument("Base strategy cannot be null");
        }
        if (max_depth == 0) {
            throw std::invalid_argument("Max depth must be greater than 0");
        }
        if (min_size == 0) {
            throw std::invalid_argument("Min size must be greater than 0");
        }
    }

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        try {
            auto result = recursive_apply(data, 0);
            if (result.empty() && !data.empty()) {
                return {data};
            }
            return result;
        } catch (const std::exception&) {
            return {data};
        }
    }
};

template <typename T>
class HierarchicalSubChunkStrategy : public ChunkStrategy<T> {
private:
    std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies_;
    size_t min_size_;

public:
    HierarchicalSubChunkStrategy(std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies,
                                 size_t min_size)
        : strategies_(std::move(strategies)), min_size_(min_size) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        if (data.empty())
            return {};
        if (data.size() <= min_size_)
            return {data};

        std::vector<std::vector<T>> current_chunks = {data};

        for (const auto& strategy : strategies_) {
            std::vector<std::vector<T>> next_level;
            for (const auto& chunk : current_chunks) {
                if (chunk.size() > min_size_) {
                    auto sub_chunks = strategy->apply(chunk);
                    next_level.insert(next_level.end(), sub_chunks.begin(), sub_chunks.end());
                } else {
                    next_level.push_back(chunk);
                }
            }
            current_chunks = std::move(next_level);
        }

        return current_chunks;
    }
};

template <typename T>
class ConditionalSubChunkStrategy : public ChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> base_strategy_;
    std::function<bool(const std::vector<T>&)> condition_;
    size_t min_size_;

public:
    ConditionalSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy,
                                std::function<bool(const std::vector<T>&)> condition,
                                size_t min_size)
        : base_strategy_(strategy), condition_(std::move(condition)), min_size_(min_size) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        if (data.empty())
            return {};
        if (data.size() <= min_size_)
            return {data};

        if (condition_(data)) {
            return base_strategy_->apply(data);
        }
        return {data};
    }
};

} // namespace chunk_processing