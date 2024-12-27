/**
 * @file chunk_metrics.hpp
 * @brief Quality metrics and analysis tools for chunk evaluation
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once
#include "chunk_common.hpp"
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
class CHUNK_EXPORT ChunkQualityAnalyzer {
public:
    /**
     * @brief Calculate cohesion (internal similarity) of chunks
     * @param chunks Vector of chunk data
     * @return Cohesion score between 0 and 1
     * @throws std::invalid_argument if chunks is empty
     */
    double compute_cohesion(const std::vector<std::vector<T>>& chunks) {
        if (chunks.empty()) {
            throw std::invalid_argument("Empty chunks vector");
        }

        double total_cohesion = 0.0;
        for (const auto& chunk : chunks) {
            if (chunk.empty())
                continue;

            T mean = calculate_mean(chunk);
            T variance = calculate_variance(chunk, mean);

            // Normalize variance to [0,1] range
            total_cohesion += 1.0 / (1.0 + std::sqrt(variance));
        }

        return total_cohesion / chunks.size();
    }

    /**
     * @brief Calculate separation (dissimilarity between chunks)
     * @param chunks Vector of chunk data
     * @return Separation score between 0 and 1
     * @throws std::invalid_argument if chunks is empty or contains single chunk
     */
    double compute_separation(const std::vector<std::vector<T>>& chunks) {
        if (chunks.size() < 2) {
            throw std::invalid_argument("Need at least two chunks for separation");
        }

        double total_separation = 0.0;
        int comparisons = 0;

        for (size_t i = 0; i < chunks.size(); ++i) {
            for (size_t j = i + 1; j < chunks.size(); ++j) {
                T mean_i = calculate_mean(chunks[i]);
                T mean_j = calculate_mean(chunks[j]);

                // Calculate distance between means
                double separation = std::abs(mean_i - mean_j);
                total_separation += separation;
                ++comparisons;
            }
        }

        return total_separation / comparisons;
    }

    /**
     * @brief Calculate silhouette score for chunk validation
     * @param chunks Vector of chunk data
     * @return Silhouette score between -1 and 1
     * @throws std::invalid_argument if chunks is empty or contains single chunk
     */
    double compute_silhouette_score(const std::vector<std::vector<T>>& chunks) {
        if (chunks.size() < 2) {
            throw std::invalid_argument("Need at least two chunks for silhouette score");
        }

        double total_score = 0.0;
        size_t total_points = 0;

        for (size_t i = 0; i < chunks.size(); ++i) {
            for (const auto& point : chunks[i]) {
                // Calculate a (average distance to points in same chunk)
                double a = 0.0;
                for (const auto& other_point : chunks[i]) {
                    if (&point != &other_point) {
                        a += std::abs(point - other_point);
                    }
                }
                a = chunks[i].size() > 1 ? a / (chunks[i].size() - 1) : 0;

                // Calculate b (minimum average distance to points in other chunks)
                double b = std::numeric_limits<double>::max();
                for (size_t j = 0; j < chunks.size(); ++j) {
                    if (i != j) {
                        double avg_dist = 0.0;
                        for (const auto& other_point : chunks[j]) {
                            avg_dist += std::abs(point - other_point);
                        }
                        avg_dist /= chunks[j].size();
                        b = std::min(b, avg_dist);
                    }
                }

                // Calculate silhouette score for this point
                double max_ab = std::max(a, b);
                if (max_ab > 0) {
                    total_score += (b - a) / max_ab;
                }
                ++total_points;
            }
        }

        return total_score / total_points;
    }

    /**
     * @brief Calculate overall quality score combining multiple metrics
     * @param chunks Vector of chunk data
     * @return Quality score between 0 and 1
     * @throws std::invalid_argument if chunks is empty
     */
    double compute_quality_score(const std::vector<std::vector<T>>& chunks) {
        if (chunks.empty()) {
            throw std::invalid_argument("Empty chunks vector");
        }

        double cohesion = compute_cohesion(chunks);
        double separation = chunks.size() > 1 ? compute_separation(chunks) : 1.0;

        return (cohesion + separation) / 2.0;
    }

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
    T calculate_mean(const std::vector<T>& chunk) {
        if (chunk.empty()) {
            return T{};
        }
        T sum = T{};
        for (const auto& val : chunk) {
            sum += val;
        }
        return sum / static_cast<T>(chunk.size());
    }

    /**
     * @brief Calculate variance of a chunk
     * @param chunk Single chunk data
     * @param mean Pre-calculated mean value
     * @return Variance of the chunk
     */
    T calculate_variance(const std::vector<T>& chunk, T mean) {
        if (chunk.size() < 2) {
            return T{};
        }
        T sum_sq_diff = T{};
        for (const auto& val : chunk) {
            T diff = val - mean;
            sum_sq_diff += diff * diff;
        }
        return sum_sq_diff / static_cast<T>(chunk.size() - 1);
    }

    // Add cache containers
    std::unordered_map<size_t, double> cached_cohesion;
    std::unordered_map<size_t, double> cached_separation;
};

} // namespace chunk_metrics