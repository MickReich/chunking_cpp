#include "chunk_metrics.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

class MetricsTest : public ::testing::Test {
protected:
    // Test data setup
    std::vector<double> sequence = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<std::vector<double>> chunks = {{1.0, 1.1, 1.2}, {2.0, 2.1, 2.2}, {4.0, 4.1, 4.2}};
    std::vector<double> empty_sequence = {};
};

// Chunk Size Distribution Tests
TEST_F(MetricsTest, ChunkSizeDistribution) {
    ChunkMetrics metrics;
    auto distribution = metrics.calculate_size_distribution(chunks);
    EXPECT_EQ(distribution.mean, 3.0);             // All chunks size 3
    EXPECT_NEAR(distribution.std_dev, 0.0, 1e-10); // Uniform sizes
}

// Chunk Consistency Tests
TEST_F(MetricsTest, ChunkConsistency) {
    ChunkMetrics metrics;
    double consistency = metrics.measure_chunk_consistency(chunks);
    EXPECT_GE(consistency, 0.0);
    EXPECT_LE(consistency, 1.0);
}

// Inter-Chunk Distance Tests
TEST_F(MetricsTest, InterChunkDistance) {
    ChunkMetrics metrics;
    auto distances = metrics.calculate_inter_chunk_distances(chunks);
    EXPECT_EQ(distances.size(), chunks.size() - 1);
    for (const auto& dist : distances) {
        EXPECT_GT(dist, 0.0);
    }
}

// Chunk Overlap Tests
TEST_F(MetricsTest, ChunkOverlap) {
    ChunkMetrics metrics;
    double overlap = metrics.calculate_chunk_overlap(chunks[0], chunks[1]);
    EXPECT_GE(overlap, 0.0);
    EXPECT_LE(overlap, 1.0);
}

// Entropy Tests
TEST_F(MetricsTest, ChunkEntropy) {
    ChunkMetrics metrics;
    double entropy = metrics.calculate_chunk_entropy(chunks[0]);
    EXPECT_GT(entropy, 0.0);
}

// Cohesion Tests
TEST_F(MetricsTest, ChunkCohesion) {
    ChunkMetrics metrics;
    double cohesion = metrics.calculate_chunk_cohesion(chunks[0]);
    EXPECT_GE(cohesion, 0.0);
    EXPECT_LE(cohesion, 1.0);
}

// Separation Tests
TEST_F(MetricsTest, ChunkSeparation) {
    ChunkMetrics metrics;
    double separation = metrics.calculate_chunk_separation(chunks);
    EXPECT_GT(separation, 0.0);
}

// Quality Score Tests
TEST_F(MetricsTest, QualityScore) {
    ChunkMetrics metrics;
    double quality = metrics.calculate_quality_score(chunks);
    EXPECT_GE(quality, 0.0);
    EXPECT_LE(quality, 1.0);
}

// Edge Cases
TEST_F(MetricsTest, EmptyChunks) {
    ChunkMetrics metrics;
    EXPECT_EQ(metrics.calculate_size_distribution({}).mean, 0.0);
    EXPECT_EQ(metrics.calculate_inter_chunk_distances({}).size(), 0);
}

TEST_F(MetricsTest, SingleChunk) {
    ChunkMetrics metrics;
    std::vector<std::vector<double>> single_chunk = {{1.0, 2.0, 3.0}};
    EXPECT_NO_THROW(metrics.calculate_quality_score(single_chunk));
}

// Performance Tests
TEST_F(MetricsTest, LargeChunkPerformance) {
    ChunkMetrics metrics;
    std::vector<std::vector<double>> large_chunks(1000, std::vector<double>(1000, 1.0));
    EXPECT_NO_THROW(metrics.calculate_quality_score(large_chunks));
}

// Statistical Tests
TEST_F(MetricsTest, ChunkStatistics) {
    ChunkMetrics metrics;
    auto stats = metrics.calculate_chunk_statistics(chunks[0]);
    EXPECT_TRUE(stats.contains("mean"));
    EXPECT_TRUE(stats.contains("variance"));
    EXPECT_TRUE(stats.contains("skewness"));
}

// Comparative Metrics
TEST_F(MetricsTest, ChunkComparison) {
    ChunkMetrics metrics;
    auto comparison = metrics.compare_chunks(chunks[0], chunks[1]);
    EXPECT_GT(comparison.similarity, 0.0);
    EXPECT_GT(comparison.distance, 0.0);
}

// Distribution Tests
TEST_F(MetricsTest, ValueDistribution) {
    ChunkMetrics metrics;
    auto distribution = metrics.analyze_value_distribution(chunks[0]);
    EXPECT_TRUE(distribution.is_normal);
    EXPECT_GT(distribution.range, 0.0);
}

// Boundary Tests
TEST_F(MetricsTest, ChunkBoundaries) {
    ChunkMetrics metrics;
    auto boundaries = metrics.analyze_chunk_boundaries(chunks);
    EXPECT_GT(boundaries.sharpness, 0.0);
    EXPECT_LE(boundaries.overlap_ratio, 1.0);
}

// Special Values
TEST_F(MetricsTest, HandleSpecialValues) {
    ChunkMetrics metrics;
    std::vector<double> special = {std::numeric_limits<double>::infinity(),
                                   -std::numeric_limits<double>::infinity(),
                                   std::numeric_limits<double>::quiet_NaN()};
    EXPECT_NO_THROW(metrics.calculate_chunk_entropy({special}));
}

// Metric Combinations
TEST_F(MetricsTest, CombinedMetrics) {
    ChunkMetrics metrics;
    auto combined = metrics.calculate_combined_metrics(chunks);
    EXPECT_TRUE(combined.contains("quality"));
    EXPECT_TRUE(combined.contains("consistency"));
    EXPECT_TRUE(combined.contains("separation"));
}

class ChunkMetricsTest : public ::testing::Test {
protected:
    void SetUp() override {
        well_separated_chunks = {
            {1.0, 1.1, 1.2}, // High cohesion
            {5.0, 5.1, 5.2}, // High cohesion
            {9.0, 9.1, 9.2}  // High cohesion
        };

        mixed_cohesion_chunks = {
            {1.0, 1.1, 1.2}, // High cohesion
            {5.0, 1.0, 9.0}, // Low cohesion
            {9.0, 9.1, 9.2}  // High cohesion
        };
    }

    std::vector<std::vector<double>> well_separated_chunks;
    std::vector<std::vector<double>> mixed_cohesion_chunks;
    chunk_metrics::ChunkQualityAnalyzer<double> analyzer;
};

TEST_F(ChunkMetricsTest, CohesionCalculation) {
    double high_cohesion = analyzer.compute_cohesion(well_separated_chunks);
    double mixed_cohesion = analyzer.compute_cohesion(mixed_cohesion_chunks);

    EXPECT_GE(high_cohesion, mixed_cohesion);
    EXPECT_GE(high_cohesion, 0.0);
    EXPECT_LE(high_cohesion, 1.0);
}

TEST_F(ChunkMetricsTest, SeparationCalculation) {
    double separation = analyzer.compute_separation(well_separated_chunks);
    EXPECT_GT(separation, 0.5); // Well separated chunks should have high separation
    EXPECT_LE(separation, 1.0);
}

TEST_F(ChunkMetricsTest, SilhouetteScore) {
    double silhouette = analyzer.compute_silhouette_score(well_separated_chunks);
    EXPECT_GE(silhouette, -1.0);
    EXPECT_LE(silhouette, 1.0);
    EXPECT_GT(silhouette, 0.0); // Well separated chunks should have positive silhouette
}

TEST_F(ChunkMetricsTest, QualityScore) {
    double high_quality = analyzer.compute_quality_score(well_separated_chunks);
    double mixed_quality = analyzer.compute_quality_score(mixed_cohesion_chunks);

    EXPECT_GT(high_quality, mixed_quality);
    EXPECT_GE(high_quality, 0.0);
    EXPECT_LE(high_quality, 1.0);
}

TEST_F(ChunkMetricsTest, EmptyChunks) {
    std::vector<std::vector<double>> empty_chunks;
    EXPECT_THROW(analyzer.compute_quality_score(empty_chunks), std::invalid_argument);
}