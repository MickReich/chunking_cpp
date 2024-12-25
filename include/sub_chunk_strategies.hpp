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

#include "chunk_common.hpp"
#include "chunk_strategies.hpp"
#include <memory>
#include <vector>

namespace chunk_processing {

template <typename T>
class RecursiveSubChunkStrategy : public ChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> strategy_;
    size_t max_depth_;
    size_t min_chunk_size_;

public:
    RecursiveSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy, size_t max_depth = 2,
                              size_t min_chunk_size = 1)
        : strategy_(strategy), max_depth_(max_depth), min_chunk_size_(min_chunk_size) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        if (data.empty() || data.size() < min_chunk_size_) {
            return {data};
        }
        return apply_recursive(data, 0);
    }

protected:
    std::vector<std::vector<T>> apply_recursive(const std::vector<T>& data, size_t depth) const {
        if (depth >= max_depth_ || data.size() < min_chunk_size_) {
            return {data};
        }
        return strategy_->apply(data);
    }
};

template <typename T>
class HierarchicalSubChunkStrategy : public ChunkStrategy<T> {
private:
    std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies_;
    size_t min_chunk_size_;

public:
    HierarchicalSubChunkStrategy(std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies,
                                 size_t min_chunk_size = 1)
        : strategies_(std::move(strategies)), min_chunk_size_(min_chunk_size) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        if (data.empty() || data.size() < min_chunk_size_) {
            return {data};
        }
        return apply_hierarchical(data, 0);
    }

protected:
    std::vector<std::vector<T>> apply_hierarchical(const std::vector<T>& data, size_t level) const {
        if (level >= strategies_.size() || data.size() < min_chunk_size_) {
            return {data};
        }
        return strategies_[level]->apply(data);
    }
};

template <typename T>
class ConditionalSubChunkStrategy : public ChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> strategy_;
    std::function<bool(const std::vector<T>&)> condition_;
    size_t min_chunk_size_;

public:
    ConditionalSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy,
                                std::function<bool(const std::vector<T>&)> condition,
                                size_t min_chunk_size = 1)
        : strategy_(strategy), condition_(condition), min_chunk_size_(min_chunk_size) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        if (data.empty() || data.size() < min_chunk_size_) {
            return {data};
        }

        if (condition_(data)) {
            return strategy_->apply(data);
        }
        return {data};
    }
};

} // namespace chunk_processing