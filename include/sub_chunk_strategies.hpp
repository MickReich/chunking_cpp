#pragma once

#include "chunk_strategies.hpp"
#include <memory>
#include <vector>

namespace chunk_strategies {

/**
 * @brief Base class for sub-chunking strategies
 * @tparam T The type of elements to process
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
        const std::vector<std::vector<T>>& chunks) override;

private:
    /**
     * @brief Helper function for recursive chunking
     * @param chunks The chunks to process
     * @param current_depth Current recursion depth
     * @return Processed sub-chunks
     */
    std::vector<std::vector<std::vector<T>>> apply_recursive(
        const std::vector<std::vector<T>>& chunks, size_t current_depth);
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
        const std::vector<std::vector<T>>& chunks) override;

private:
    /**
     * @brief Helper function for hierarchical chunking
     * @param chunks The chunks to process
     * @param level Current hierarchy level
     * @return Processed sub-chunks
     */
    std::vector<std::vector<std::vector<T>>> apply_hierarchical(
        const std::vector<std::vector<T>>& chunks, size_t level);
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
        const std::vector<std::vector<T>>& chunks) override;
};

} // namespace chunk_strategies 
