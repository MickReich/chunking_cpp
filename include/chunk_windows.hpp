#pragma once

#include "chunk.hpp"
#include <functional>
#include <vector>

namespace chunk_windows {

template <typename T>
class SlidingWindowProcessor {
public:
    SlidingWindowProcessor(size_t window_size, size_t step_size)
        : window_size_(window_size), step_size_(step_size) {}

    std::vector<T> process(const std::vector<T>& data,
                           std::function<T(const std::vector<T>&)> aggregator) {
        Chunk<T> chunker(window_size_);
        chunker.add(data);
        auto windows = chunker.sliding_window(window_size_, step_size_);

        std::vector<T> results;
        for (const auto& window : windows) {
            results.push_back(aggregator(window));
        }
        return results;
    }

private:
    size_t window_size_;
    size_t step_size_;
};

// Common window operations
template <typename T>
class WindowOperations {
public:
    static T moving_average(const std::vector<T>& window) {
        return std::accumulate(window.begin(), window.end(), T{}) / static_cast<T>(window.size());
    }

    static T moving_median(std::vector<T> window) {
        std::sort(window.begin(), window.end());
        return window[window.size() / 2];
    }

    static T moving_max(const std::vector<T>& window) {
        return *std::max_element(window.begin(), window.end());
    }

    static T moving_min(const std::vector<T>& window) {
        return *std::min_element(window.begin(), window.end());
    }
};

} // namespace chunk_windows