#include "chunk_resilience.hpp"
#include <gtest/gtest.h>

class ResilientChunkerTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = "./test_checkpoint";
        std::filesystem::create_directories(test_dir);
        data = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }

    std::string test_dir;
    std::vector<double> data;
};

TEST_F(ResilientChunkerTest, BasicProcessing) {
    chunk_resilience::ResilientChunker<double> chunker(test_dir, 1024 * 1024, 2, 1);
    auto result = chunker.process(data);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.size(), (data.size() + 1) / 2);
}

TEST_F(ResilientChunkerTest, CheckpointCreation) {
    chunk_resilience::ResilientChunker<double> chunker(test_dir, 1024 * 1024, 2, 1);
    chunker.process(data);
    chunker.save_checkpoint();

    EXPECT_TRUE(std::filesystem::exists(test_dir + "/latest_checkpoint.bin"));
}

TEST_F(ResilientChunkerTest, CheckpointRestoration) {
    chunk_resilience::ResilientChunker<double> chunker(test_dir, 1024 * 1024, 2, 1);
    auto original = chunker.process(data);
    chunker.save_checkpoint();

    auto restored = chunker.restore_from_checkpoint();
    EXPECT_EQ(original.size(), restored.size());
}

TEST_F(ResilientChunkerTest, MemoryExhaustion) {
    // Set very low memory limit to trigger exhaustion
    chunk_resilience::ResilientChunker<double> chunker(test_dir, 1, 2, 1);
    EXPECT_THROW(chunker.process(data), chunk_resilience::ChunkingError);
}

TEST_F(ResilientChunkerTest, EmptyInput) {
    chunk_resilience::ResilientChunker<double> chunker(test_dir, 1024 * 1024, 2, 1);
    auto result = chunker.process(std::vector<double>());
    EXPECT_TRUE(result.empty());
}