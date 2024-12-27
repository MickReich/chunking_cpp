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
    static constexpr size_t MAX_CHUNKS = 1000;

    struct RecursionState {
        size_t chunk_count = 0;
        bool limit_reached = false;
    };

    std::vector<std::vector<T>> recursive_apply(const std::vector<T>& data, 
                                              size_t depth,
                                              RecursionState& state) const {
        // Early exit conditions
        if (data.empty() || state.limit_reached) {
            return {data};
        }

        if (data.size() <= min_size_ || depth >= max_depth_) {
            return {data};
        }

        try {
            // Apply base strategy
            auto chunks = base_strategy_->apply(data);
            
            // Validate base strategy output
            if (chunks.empty() || chunks.size() == 1) {
                return {data};
            }

            // Validate chunk sizes
            for (const auto& chunk : chunks) {
                if (chunk.empty() || chunk.size() < min_size_) {
                    return {data};  // Reject invalid splits
                }
            }

            // Verify total elements
            size_t total_elements = 0;
            for (const auto& chunk : chunks) {
                total_elements += chunk.size();
            }
            if (total_elements != data.size()) {
                return {data};  // Reject if elements were lost/added
            }

            // Process sub-chunks
            std::vector<std::vector<T>> result;
            result.reserve(chunks.size());

            for (const auto& chunk : chunks) {
                if (state.chunk_count >= MAX_CHUNKS) {
                    state.limit_reached = true;
                    result.push_back(chunk);
                    continue;
                }

                auto sub_chunks = recursive_apply(chunk, depth + 1, state);
                
                // Track new chunks
                state.chunk_count += sub_chunks.size();
                
                // Append valid sub-chunks
                for (const auto& sub_chunk : sub_chunks) {
                    if (!sub_chunk.empty() && sub_chunk.size() >= min_size_) {
                        result.push_back(sub_chunk);
                    }
                }
            }

            return result.empty() ? std::vector<std::vector<T>>{{data}} : result;

        } catch (const std::exception&) {
            return {data};  // Safely handle any exceptions
        }
    }

public:
    RecursiveSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy,
                             size_t max_depth,
                             size_t min_size)
        : base_strategy_(strategy)
        , max_depth_(max_depth)
        , min_size_(min_size) {
        if (!strategy) {
            throw std::invalid_argument("Base strategy cannot be null");
        }
        if (max_depth == 0 || max_depth > 100) {
            throw std::invalid_argument("Invalid max depth");
        }
        if (min_size == 0) {
            throw std::invalid_argument("Min size must be greater than 0");
        }
    }

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        if (data.empty()) {
            return {};
        }
        
        if (data.size() <= min_size_) {
            return {data};
        }

        RecursionState state;
        auto result = recursive_apply(data, 0, state);
        
        // Final validation
        if (result.empty()) {
            return {data};
        }
        
        // Ensure all chunks meet minimum size
        for (const auto& chunk : result) {
            if (chunk.size() < min_size_) {
                return {data};  // If any chunk is too small, return original
            }
        }

        return result;
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