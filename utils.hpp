#ifndef UTILS_HPP
#define UTILS_HPP

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <functional>
#include <random>

namespace chunk_utils {

// Statistics utilities
template<typename T>
class Statistics {
public:
    static T mean(const std::vector<T>& data) {
        if (data.empty()) return T();
        return std::accumulate(data.begin(), data.end(), T()) / static_cast<T>(data.size());
    }

    static T median(std::vector<T> data) {
        if (data.empty()) return T();
        std::sort(data.begin(), data.end());
        return data[data.size() / 2];
    }

    static std::pair<T, size_t> mode(const std::vector<T>& data) {
        if (data.empty()) return {T(), 0};
        std::map<T, size_t> freq;
        for (const auto& val : data) freq[val]++;
        return *std::max_element(freq.begin(), freq.end(),
            [](const auto& p1, const auto& p2) { return p1.second < p2.second; });
    }
};

// Chunk manipulation utilities
template<typename T>
class ChunkManipulator {
public:
    static std::vector<std::vector<T>> merge_chunks(
        const std::vector<std::vector<T>>& chunks1,
        const std::vector<std::vector<T>>& chunks2) {
        std::vector<std::vector<T>> result = chunks1;
        result.insert(result.end(), chunks2.begin(), chunks2.end());
        return result;
    }

    static std::vector<std::vector<T>> filter_chunks(
        const std::vector<std::vector<T>>& chunks,
        std::function<bool(const std::vector<T>&)> predicate) {
        std::vector<std::vector<T>> result;
        std::copy_if(chunks.begin(), chunks.end(), std::back_inserter(result), predicate);
        return result;
    }

    static std::vector<std::vector<T>> transform_chunks(
        const std::vector<std::vector<T>>& chunks,
        std::function<std::vector<T>(const std::vector<T>&)> transformer) {
        std::vector<std::vector<T>> result;
        std::transform(chunks.begin(), chunks.end(), std::back_inserter(result), transformer);
        return result;
    }
};

// Random chunk generation
template<typename T>
class ChunkGenerator {
private:
    static std::random_device rd;
    static std::mt19937 gen;

public:
    static std::vector<T> generate_random_data(size_t size, T min_val, T max_val) {
        std::vector<T> data(size);
        std::uniform_real_distribution<T> dis(min_val, max_val);
        std::generate(data.begin(), data.end(), [&]() { return dis(gen); });
        return data;
    }

    static std::vector<std::vector<T>> generate_random_chunks(
        size_t num_chunks, size_t chunk_size, T min_val, T max_val) {
        std::vector<std::vector<T>> chunks(num_chunks);
        for (auto& chunk : chunks) {
            chunk = generate_random_data(chunk_size, min_val, max_val);
        }
        return chunks;
    }
};

template<typename T>
std::random_device ChunkGenerator<T>::rd;
template<typename T>
std::mt19937 ChunkGenerator<T>::gen(ChunkGenerator<T>::rd());

} // namespace chunk_utils

#endif // UTILS_HPP 