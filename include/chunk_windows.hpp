#pragma once

#include "chunk.hpp"
#include <functional>
#include <vector>
#include <numeric>
#include <algorithm>

namespace chunk_windows {

template <typename T>
class SlidingWindowProcessor {
private:
    size_t window_size_;
    size_t step_size_;

    std::vector<T> process_chunks(const std::vector<std::vector<T>>& chunks,
                                std::function<T(const std::vector<T>&)> window_func) {
        std::vector<T> results;
        for (size_t i = 0; i < chunks.size(); i += step_size_) {
            const auto& chunk = chunks[i];
            if (chunk.size() >= window_size_) {
                results.push_back(window_func(chunk));
            }
        }
        return results;
    }

public:
    SlidingWindowProcessor(size_t window_size, size_t step_size)
        : window_size_(window_size), step_size_(step_size) {}

    std::vector<T> process(const std::vector<T>& data,
                          std::function<T(const std::vector<T>&)> window_func) {
        chunk_processing::Chunk<T> chunker(window_size_);
        chunker.add(data);
        return process_chunks(chunker.get_chunks(), window_func);
    }
};

// Common window operations
template <typename T>
class WindowOperations {
public:
    static T moving_average(const std::vector<T>& window) {
        if (window.empty()) {
            return T{}; // Return default value for empty input
        }
        return std::accumulate(window.begin(), window.end(), T{}) / static_cast<T>(window.size());
    }

    static T moving_median(std::vector<T> window) {
        if (window.empty()) {
            throw std::invalid_argument("Cannot compute median of empty window");
        }
        std::sort(window.begin(), window.end());
        return window[window.size() / 2];
    }

    static T moving_max(const std::vector<T>& window) {
        if (window.empty()) {
            throw std::invalid_argument("Cannot compute max of empty window");
        }
        return *std::max_element(window.begin(), window.end());
    }

    static T moving_min(const std::vector<T>& window) {
        if (window.empty()) {
            throw std::invalid_argument("Cannot compute min of empty window");
        }
        return *std::min_element(window.begin(), window.end());
    }
};

} // namespace chunk_windows