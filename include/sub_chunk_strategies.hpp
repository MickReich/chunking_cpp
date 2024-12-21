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
#include <memory>
#include <vector>

namespace chunk_strategies {

/**
 * @brief Base class for sub-chunking strategies
 * @tparam T The type of elements to process
 * 
 * This abstract class defines the interface for all sub-chunking strategies.
 * Sub-chunking strategies operate on chunks of data to produce hierarchical
 * or nested chunk structures.
 */
template <typename T>
class SubChunkStrategy {
public:
    /**
     * @brief Apply the sub-chunking strategy to a set of chunks
     * @param chunks The input chunks to process
     * @return A vector of vectors of vectors containing the sub-chunked data
     */
    virtual std::vector<std::vector<std::vector<T>>> apply(
        const std::vector<std::vector<T>>& chunks) = 0;
    
    /**
     * @brief Virtual destructor for proper cleanup
     */
    virtual ~SubChunkStrategy() = default;
};

/**
 * @brief Strategy for applying recursive sub-chunking
 * @tparam T The type of elements to process
 */
template <typename T>
class RecursiveSubChunkStrategy : public SubChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> strategy_;
    size_t max_depth_;
    size_t min_chunk_size_;

public:
    /**
     * @brief Construct a recursive sub-chunking strategy
     * @param strategy The base chunking strategy to apply recursively
     * @param max_depth Maximum recursion depth
     * @param min_chunk_size Minimum size for chunks to be processed
     */
    RecursiveSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy,
                            size_t max_depth = 2,
                            size_t min_chunk_size = 1)
        : strategy_(strategy), max_depth_(max_depth), min_chunk_size_(min_chunk_size) {}

    /**
     * @brief Apply recursive sub-chunking to the input chunks
     * @param chunks The input chunks to process
     * @return A vector of vectors of vectors containing the recursively sub-chunked data
     */
    std::vector<std::vector<std::vector<T>>> apply(
        const std::vector<std::vector<T>>& chunks) override {
        return apply_recursive(chunks, 1);
    }

private:
    /**
     * @brief Helper function for recursive chunking
     * @param chunks The chunks to process
     * @param current_depth Current recursion depth
     * @return Processed sub-chunks
     */
    std::vector<std::vector<std::vector<T>>> apply_recursive(
        const std::vector<std::vector<T>>& chunks, size_t current_depth) {
        /**
         * @details This method recursively applies the chunking strategy:
         * 1. Checks depth and size constraints
         * 2. Applies base strategy to each chunk
         * 3. Recursively processes sub-chunks if needed
         * 4. Returns the hierarchical result
         */
        std::vector<std::vector<std::vector<T>>> result;
        
        for (const auto& chunk : chunks) {
            if (current_depth >= max_depth_ || chunk.size() <= min_chunk_size_) {
                result.push_back({chunk});
                continue;
            }

            auto sub_chunks = strategy_->apply(chunk);
            if (current_depth < max_depth_) {
                auto recursive_result = apply_recursive(sub_chunks, current_depth + 1);
                result.insert(result.end(), recursive_result.begin(), recursive_result.end());
            } else {
                result.push_back(sub_chunks);
            }
        }

        return result;
    }
};

/**
 * @brief Strategy for hierarchical sub-chunking using multiple strategies
 * @tparam T The type of elements to process
 */
template <typename T>
class HierarchicalSubChunkStrategy : public SubChunkStrategy<T> {
private:
    std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies_;
    size_t min_chunk_size_;

public:
    /**
     * @brief Construct a hierarchical sub-chunking strategy
     * @param strategies Vector of strategies to apply at different levels
     * @param min_chunk_size Minimum size for chunks to be processed
     */
    HierarchicalSubChunkStrategy(
        std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies,
        size_t min_chunk_size = 1)
        : strategies_(strategies), min_chunk_size_(min_chunk_size) {}

    /**
     * @brief Apply hierarchical sub-chunking to the input chunks
     * @param chunks The input chunks to process
     * @return A vector of vectors of vectors containing the hierarchically sub-chunked data
     */
    std::vector<std::vector<std::vector<T>>> apply(
        const std::vector<std::vector<T>>& chunks) override {
        return apply_hierarchical(chunks, 0);
    }

private:
    /**
     * @brief Helper function for hierarchical chunking
     * @param chunks The chunks to process
     * @param level Current hierarchy level
     * @return Processed sub-chunks
     */
    std::vector<std::vector<std::vector<T>>> apply_hierarchical(
        const std::vector<std::vector<T>>& chunks, size_t level) {
        std::vector<std::vector<std::vector<T>>> result;

        if (level >= strategies_.size()) {
            result.push_back(chunks);
            return result;
        }

        for (const auto& chunk : chunks) {
            if (chunk.size() <= min_chunk_size_) {
                result.push_back({chunk});
                continue;
            }

            auto sub_chunks = strategies_[level]->apply(chunk);
            auto next_level_chunks = apply_hierarchical(sub_chunks, level + 1);
            result.insert(result.end(), next_level_chunks.begin(), next_level_chunks.end());
        }

        return result;
    }
};

/**
 * @brief Strategy for conditional sub-chunking based on chunk properties
 * @tparam T The type of elements to process
 */
template <typename T>
class ConditionalSubChunkStrategy : public SubChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> strategy_;
    std::function<bool(const std::vector<T>&)> condition_;
    size_t min_chunk_size_;

public:
    /**
     * @brief Construct a conditional sub-chunking strategy
     * @param strategy The base chunking strategy to apply
     * @param condition Function determining when to apply sub-chunking
     * @param min_chunk_size Minimum size for chunks to be processed
     */
    ConditionalSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy,
                              std::function<bool(const std::vector<T>&)> condition,
                              size_t min_chunk_size = 1)
        : strategy_(strategy), condition_(condition), min_chunk_size_(min_chunk_size) {}

    /**
     * @brief Apply conditional sub-chunking to the input chunks
     * @param chunks The input chunks to process
     * @return A vector of vectors of vectors containing the conditionally sub-chunked data
     */
    std::vector<std::vector<std::vector<T>>> apply(
        const std::vector<std::vector<T>>& chunks) override {
        std::vector<std::vector<std::vector<T>>> result;

        for (const auto& chunk : chunks) {
            if (chunk.size() <= min_chunk_size_ || !condition_(chunk)) {
                result.push_back({chunk});
                continue;
            }

            auto sub_chunks = strategy_->apply(chunk);
            result.push_back(sub_chunks);
        }

        return result;
    }
};

} // namespace chunk_strategies 