#include <gtest/gtest.h>
#include "../include/chunk.hpp"

class ChunkTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1, 2, 3, 4, 5};
    }

    std::vector<int> test_data;
    Chunk<int> basic_chunker{2};
};

TEST_F(ChunkTest, Constructor) {
    EXPECT_THROW(Chunk<int>(0), std::invalid_argument);
    EXPECT_NO_THROW(Chunk<int>(1));
}

// ... rest of the tests ... 