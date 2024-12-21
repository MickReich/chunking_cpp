#pragma once

#include "chunk_strategies.hpp"
#include <memory>
#include <vector>

namespace chunk_strategies {

template <typename T>
class SubChunkStrategy {
public:
    virtual std::vector<std::vector<std::vector<T>>> apply(
        const std::vector<std::vector<T>>& chunks) = 0;
    virtual ~SubChunkStrategy() = default;
};

template <typename T>
class RecursiveSubChunkStrategy : public SubChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> strategy_;
    size_t max_depth_;
    size_t min_chunk_size_;

public:
    RecursiveSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy,
                            size_t max_depth = 2,
                            size_t min_chunk_size = 1)
        : strategy_(strategy), max_depth_(max_depth), min_chunk_size_(min_chunk_size) {}

    std::vector<std::vector<std::vector<T>>> apply(
        const std::vector<std::vector<T>>& chunks) override {
        return apply_recursive(chunks, 1);
    }

private:
    std::vector<std::vector<std::vector<T>>> apply_recursive(
        const std::vector<std::vector<T>>& chunks, size_t current_depth) {
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

template <typename T>
class HierarchicalSubChunkStrategy : public SubChunkStrategy<T> {
private:
    std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies_;
    size_t min_chunk_size_;

public:
    HierarchicalSubChunkStrategy(
        std::vector<std::shared_ptr<ChunkStrategy<T>>> strategies,
        size_t min_chunk_size = 1)
        : strategies_(strategies), min_chunk_size_(min_chunk_size) {}

    std::vector<std::vector<std::vector<T>>> apply(
        const std::vector<std::vector<T>>& chunks) override {
        return apply_hierarchical(chunks, 0);
    }

private:
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

template <typename T>
class ConditionalSubChunkStrategy : public SubChunkStrategy<T> {
private:
    std::shared_ptr<ChunkStrategy<T>> strategy_;
    std::function<bool(const std::vector<T>&)> condition_;
    size_t min_chunk_size_;

public:
    ConditionalSubChunkStrategy(std::shared_ptr<ChunkStrategy<T>> strategy,
                              std::function<bool(const std::vector<T>&)> condition,
                              size_t min_chunk_size = 1)
        : strategy_(strategy), condition_(condition), min_chunk_size_(min_chunk_size) {}

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