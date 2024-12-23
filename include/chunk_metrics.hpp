/**
 * @file chunk_metrics.hpp
 * @brief Quality metrics and analysis tools for chunk evaluation
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace chunk_metrics {

/**
 * @brief Class for analyzing and evaluating chunk quality
 * @tparam T The data type of the chunks (must support arithmetic operations)
 */
template <typename T>
class ChunkQualityAnalyzer {
public:
    /**
     * @brief Calculate cohesion (internal similarity) of chunks
     * @param chunks Vector of chunk data
     * @return Cohesion score between 0 and 1
     * @throws std::invalid_argument if chunks is empty
     */
    double compute_cohesion(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Calculate separation (dissimilarity between chunks)
     * @param chunks Vector of chunk data
     * @return Separation score between 0 and 1
     * @throws std::invalid_argument if chunks is empty or contains single chunk
     */
    double compute_separation(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Calculate silhouette score for chunk validation
     * @param chunks Vector of chunk data
     * @return Silhouette score between -1 and 1
     * @throws std::invalid_argument if chunks is empty or contains single chunk
     */
    double compute_silhouette_score(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Calculate overall quality score combining multiple metrics
     * @param chunks Vector of chunk data
     * @return Quality score between 0 and 1
     * @throws std::invalid_argument if chunks is empty
     */
    double compute_quality_score(const std::vector<std::vector<T>>& chunks);

    /**
     * @brief Compute size-based metrics for chunks
     * @param chunks Vector of chunk data
     * @return Map of metric names to values
     */
    std::unordered_map<std::string, double>
    compute_size_metrics(const std::vector<std::vector<T>>& chunks) {
        std::unordered_map<std::string, double> metrics;

        if (chunks.empty()) {
            throw std::invalid_argument("Empty chunks vector");
        }

        // Calculate average chunk size
        double avg_size = 0.0;
        double max_size = 0.0;
        double min_size = chunks[0].size();

        for (const auto& chunk : chunks) {
            size_t size = chunk.size();
            avg_size += size;
            max_size = std::max(max_size, static_cast<double>(size));
            min_size = std::min(min_size, static_cast<double>(size));
        }
        avg_size /= chunks.size();

        // Calculate size variance
        double variance = 0.0;
        for (const auto& chunk : chunks) {
            double diff = chunk.size() - avg_size;
            variance += diff * diff;
        }
        variance /= chunks.size();

        metrics["average_size"] = avg_size;
        metrics["max_size"] = max_size;
        metrics["min_size"] = min_size;
        metrics["size_variance"] = variance;
        metrics["size_stddev"] = std::sqrt(variance);

        return metrics;
    }

    /**
     * @brief Clear internal caches to free memory
     */
    void clear_cache() {
        // Clear any cached computations
        cached_cohesion.clear();
        cached_separation.clear();
    }

private:
    /**
     * @brief Calculate mean value of a chunk
     * @param chunk Single chunk data
     * @return Mean value of the chunk
     */
    T calculate_mean(const std::vector<T>& chunk);

    /**
     * @brief Calculate variance of a chunk
     * @param chunk Single chunk data
     * @param mean Pre-calculated mean value
     * @return Variance of the chunk
     */
    T calculate_variance(const std::vector<T>& chunk, T mean);

    // Add cache containers
    std::unordered_map<size_t, double> cached_cohesion;
    std::unordered_map<size_t, double> cached_separation;
};

} // namespace chunk_metrics