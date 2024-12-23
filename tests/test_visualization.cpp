#include "chunk_visualization.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class ChunkVisualizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = "./test_viz";
        std::filesystem::create_directories(test_dir);
        chunks = {{1.0, 2.0, 3.0}, {4.0, 5.0}, {6.0, 7.0, 8.0, 9.0}};
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }

    std::string test_dir;
    std::vector<std::vector<double>> chunks;
};

TEST_F(ChunkVisualizerTest, DirectoryCreation) {
    chunk_viz::ChunkVisualizer<std::vector<double>> viz(chunks, test_dir);
    EXPECT_TRUE(std::filesystem::exists(test_dir));
}

TEST_F(ChunkVisualizerTest, PlotChunkSizes) {
    chunk_viz::ChunkVisualizer<std::vector<double>> viz(chunks, test_dir);
    viz.plot_chunk_sizes();

    EXPECT_TRUE(std::filesystem::exists(test_dir + "/chunk_sizes.dat"));
    EXPECT_TRUE(std::filesystem::exists(test_dir + "/plot_chunks.gnu"));

    // Verify data file contents
    std::ifstream data_file(test_dir + "/chunk_sizes.dat");
    std::vector<size_t> sizes;
    size_t index, size;
    while (data_file >> index >> size) {
        sizes.push_back(size);
    }
    EXPECT_EQ(sizes, std::vector<size_t>{3, 2, 4});
}

TEST_F(ChunkVisualizerTest, BoundaryVisualization) {
    chunk_viz::ChunkVisualizer<std::vector<double>> viz(chunks, test_dir);
    viz.visualize_boundaries();

    std::string boundary_file = test_dir + "/boundaries.txt";
    EXPECT_TRUE(std::filesystem::exists(boundary_file));

    std::ifstream file(boundary_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("Total size: 9 elements") != std::string::npos);
}

TEST_F(ChunkVisualizerTest, GraphVizExport) {
    chunk_viz::ChunkVisualizer<std::vector<double>> viz(chunks, test_dir);
    viz.export_to_graphviz();

    std::string dot_file = test_dir + "/chunks.dot";
    EXPECT_TRUE(std::filesystem::exists(dot_file));

    std::ifstream file(dot_file);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("digraph chunks") != std::string::npos);
}