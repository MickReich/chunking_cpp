#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

namespace chunk_metrics {

/**
 * @brief Statistical metrics for chunk analysis
 */
template<typename T>
class ChunkQualityAnalyzer {
private:
    // Cache for computed metrics
    mutable std::unordered_map<std::string, double> metrics_cache;
    
    /**
     * @brief Calculate the centroid of a chunk
     */
    static T calculate_centroid(const std::vector<T>& chunk) {
        if (chunk.empty()) {
            throw std::invalid_argument("Cannot calculate centroid of empty chunk");
        }
        return std::accumulate(chunk.begin(), chunk.end(), T{}) / 
               static_cast<T>(chunk.size());
    }
    
    /**
     * @brief Calculate Euclidean distance between two points
     */
    static double euclidean_distance(const T& a, const T& b) {
        return std::abs(static_cast<double>(a - b));
    }
    
    /**
     * @brief Calculate variance of a chunk
     */
    static double calculate_variance(const std::vector<T>& chunk) {
        if (chunk.size() < 2) {
            return 0.0;
        }
        
        T mean = calculate_centroid(chunk);
        double sum_squares = 0.0;
        
        for (const auto& value : chunk) {
            double diff = static_cast<double>(value - mean);
            sum_squares += diff * diff;
        }
        
        return sum_squares / (chunk.size() - 1);
    }

public:
    /**
     * @brief Compute cohesion (intra-chunk similarity)
     */
    double compute_cohesion(const std::vector<T>& chunk) {
        if (chunk.size() < 2) return 1.0;
        
        const std::string cache_key = "cohesion_" + std::to_string(chunk.size());
        if (metrics_cache.count(cache_key)) {
            return metrics_cache[cache_key];
        }

        T centroid = calculate_centroid(chunk);
        double total_distance = 0.0;
        
        for (const auto& value : chunk) {
            total_distance += euclidean_distance(value, centroid);
        }
        
        double cohesion = 1.0 / (1.0 + (total_distance / chunk.size()));
        metrics_cache[cache_key] = cohesion;
        return cohesion;
    }

    /**
     * @brief Compute separation (inter-chunk dissimilarity)
     */
    double compute_separation(const std::vector<std::vector<T>>& chunks) {
        if (chunks.size() < 2) return 1.0;
        
        const std::string cache_key = "separation_" + std::to_string(chunks.size());
        if (metrics_cache.count(cache_key)) {
            return metrics_cache[cache_key];
        }

        std::vector<T> centroids;
        centroids.reserve(chunks.size());
        
        for (const auto& chunk : chunks) {
            centroids.push_back(calculate_centroid(chunk));
        }

        double min_distance = std::numeric_limits<double>::max();
        
        for (size_t i = 0; i < centroids.size(); ++i) {
            for (size_t j = i + 1; j < centroids.size(); ++j) {
                double distance = euclidean_distance(centroids[i], centroids[j]);
                min_distance = std::min(min_distance, distance);
            }
        }

        double separation = 1.0 - (1.0 / (1.0 + min_distance));
        metrics_cache[cache_key] = separation;
        return separation;
    }

    /**
     * @brief Compute silhouette score for chunk quality
     */
    double compute_silhouette_score(const std::vector<std::vector<T>>& chunks) {
        if (chunks.size() < 2) return 0.0;
        
        const std::string cache_key = "silhouette_" + std::to_string(chunks.size());
        if (metrics_cache.count(cache_key)) {
            return metrics_cache[cache_key];
        }

        double total_silhouette = 0.0;
        size_t total_points = 0;

        for (size_t i = 0; i < chunks.size(); ++i) {
            const auto& current_chunk = chunks[i];
            if (current_chunk.empty()) continue;

            for (const auto& point : current_chunk) {
                // Calculate a (average distance to points in same chunk)
                double a = 0.0;
                for (const auto& other_point : current_chunk) {
                    if (&point != &other_point) {
                        a += euclidean_distance(point, other_point);
                    }
                }
                a = current_chunk.size() > 1 ? a / (current_chunk.size() - 1) : 0;

                // Calculate b (minimum average distance to points in other chunks)
                double b = std::numeric_limits<double>::max();
                for (size_t j = 0; j < chunks.size(); ++j) {
                    if (i != j && !chunks[j].empty()) {
                        double avg_distance = 0.0;
                        for (const auto& other_point : chunks[j]) {
                            avg_distance += euclidean_distance(point, other_point);
                        }
                        avg_distance /= chunks[j].size();
                        b = std::min(b, avg_distance);
                    }
                }

                // Calculate silhouette
                if (b == std::numeric_limits<double>::max()) b = a;
                double max_ab = std::max(a, b);
                double silhouette = max_ab == 0 ? 0 : (b - a) / max_ab;
                
                total_silhouette += silhouette;
                total_points++;
            }
        }

        double avg_silhouette = total_points > 0 ? total_silhouette / total_points : 0;
        metrics_cache[cache_key] = avg_silhouette;
        return avg_silhouette;
    }

    /**
     * @brief Compute chunk size distribution metrics
     */
    struct SizeMetrics {
        double mean;
        double variance;
        size_t min_size;
        size_t max_size;
        double entropy;
    };

    SizeMetrics compute_size_metrics(const std::vector<std::vector<T>>& chunks) {
        if (chunks.empty()) {
            throw std::invalid_argument("Cannot compute metrics for empty chunks");
        }

        std::vector<size_t> sizes;
        sizes.reserve(chunks.size());
        
        for (const auto& chunk : chunks) {
            sizes.push_back(chunk.size());
        }

        // Calculate basic statistics
        double mean = std::accumulate(sizes.begin(), sizes.end(), 0.0) / sizes.size();
        
        double variance = 0.0;
        for (size_t size : sizes) {
            double diff = size - mean;
            variance += diff * diff;
        }
        variance /= sizes.size();

        // Calculate size distribution entropy
        std::unordered_map<size_t, size_t> size_counts;
        for (size_t size : sizes) {
            size_counts[size]++;
        }

        double entropy = 0.0;
        for (const auto& [size, count] : size_counts) {
            double p = static_cast<double>(count) / sizes.size();
            entropy -= p * std::log2(p);
        }

        return {
            mean,
            variance,
            *std::min_element(sizes.begin(), sizes.end()),
            *std::max_element(sizes.begin(), sizes.end()),
            entropy
        };
    }

    /**
     * @brief Compute overall chunk quality score
     */
    double compute_quality_score(const std::vector<std::vector<T>>& chunks) {
        if (chunks.empty()) return 0.0;
        
        // Weighted combination of different metrics
        double cohesion_weight = 0.3;
        double separation_weight = 0.3;
        double silhouette_weight = 0.4;
        
        double avg_cohesion = 0.0;
        for (const auto& chunk : chunks) {
            avg_cohesion += compute_cohesion(chunk);
        }
        avg_cohesion /= chunks.size();

        double separation = compute_separation(chunks);
        double silhouette = compute_silhouette_score(chunks);

        return (avg_cohesion * cohesion_weight) +
               (separation * separation_weight) +
               (silhouette * silhouette_weight);
    }

    /**
     * @brief Clear metrics cache
     */
    void clear_cache() {
        metrics_cache.clear();
    }
};

} // namespace chunk_metrics 