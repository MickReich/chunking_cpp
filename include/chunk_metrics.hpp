namespace chunk_metrics {
    template<typename T>
    class ChunkQualityAnalyzer {
        double compute_cohesion(const std::vector<T>& chunk);
        double compute_separation(const std::vector<std::vector<T>>& chunks);
        double compute_silhouette_score();
    };
} 