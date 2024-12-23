#include "chunk_metrics.hpp"
#include <gtest/gtest.h>

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