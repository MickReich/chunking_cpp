#pragma once

#include "chunk_strategies.hpp"
#include "neural_chunking.hpp"
#include "chunk.hpp"

// Strategy wrapper for neural chunking
template <typename T>
class NeuralChunkingStrategy : public chunk_strategies::ChunkStrategy<T> {
private:
    mutable neural_chunking::NeuralChunking<T> neural_chunker;

public:
    NeuralChunkingStrategy() : neural_chunker(8, 0.5) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        return neural_chunker.chunk(data);
    }
};

// Strategy wrapper for similarity-based chunking
template <typename T>
class SimilarityChunkingStrategy : public chunk_strategies::ChunkStrategy<T> {
public:
    explicit SimilarityChunkingStrategy(double threshold) : threshold(threshold) {}

    std::vector<std::vector<T>> apply(const std::vector<T>& data) const override {
        chunk_processing::Chunk<T> chunker(1);
        for (const auto& item : data) {
            chunker.add(item);
        }
        return chunker.chunk_by_threshold(threshold);
    }

private:
    double threshold;
};