#pragma once

#if defined(_MSC_VER)
#define CHUNK_EXPORT __declspec(dllexport)
#else
#define CHUNK_EXPORT __attribute__((visibility("default")))
#endif

#include <algorithm>
#include <type_traits>
#include <vector>

namespace chunk_processing {

// Type traits
template <typename T>
struct is_vector : std::false_type {};

template <typename T, typename A>
struct is_vector<std::vector<T, A>> : std::true_type {};

// Helper function to check if a vector is jagged
template <typename T>
bool is_jagged(const std::vector<std::vector<T>>& data) {
    if (data.empty())
        return false;
    const size_t expected_size = data[0].size();
    return std::any_of(data.begin(), data.end(),
                       [expected_size](const auto& row) { return row.size() != expected_size; });
}

// Helper function for 3D jagged detection
template <typename T>
bool is_jagged_3d(const std::vector<std::vector<std::vector<T>>>& data) {
    if (data.empty())
        return false;

    // Check first level consistency
    const size_t first_size = data[0].size();
    if (std::any_of(data.begin(), data.end(),
                    [first_size](const auto& matrix) { return matrix.size() != first_size; })) {
        return true;
    }

    // Check second level consistency
    const size_t second_size = data[0][0].size();
    return std::any_of(data.begin(), data.end(), [second_size](const auto& matrix) {
        return std::any_of(matrix.begin(), matrix.end(),
                           [second_size](const auto& row) { return row.size() != second_size; });
    });
}

} // namespace chunk_processing