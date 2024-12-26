#ifndef CHUNK_PROCESSING_JAGGED_ARRAY_TEST_HPP
#define CHUNK_PROCESSING_JAGGED_ARRAY_TEST_HPP

#include "chunk.hpp"
#include <gtest/gtest.h>
#include <vector>

using namespace chunk_processing;

class JaggedArrayTest : public ::testing::Test {
protected:
    // 2D jagged array
    std::vector<std::vector<double>> jagged_2d = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0},
        {6.0, 7.0, 8.0, 9.0}
    };

    // 3D jagged array
    std::vector<std::vector<std::vector<double>>> jagged_3d = {
        {
            {1.0, 2.0},
            {3.0}
        },
        {
            {4.0, 5.0, 6.0},
            {7.0, 8.0}
        }
    };

    // Expected normalized 2D array
    std::vector<std::vector<double>> expected_normalized_2d = {
        {1.0, 2.0, 3.0, 0.0},
        {4.0, 5.0, 0.0, 0.0},
        {6.0, 7.0, 8.0, 9.0}
    };

    // Expected normalized 3D array
    std::vector<std::vector<std::vector<double>>> expected_normalized_3d = {
        {
            {1.0, 2.0, 0.0},
            {3.0, 0.0, 0.0}
        },
        {
            {4.0, 5.0, 6.0},
            {7.0, 8.0, 0.0}
        }
    };
};

TEST_F(JaggedArrayTest, DetectJagged2DArray) {
    EXPECT_TRUE(is_jagged(jagged_2d));
    
    std::vector<std::vector<double>> uniform_2d = {
        {1.0, 2.0, 3.0},
        {4.0, 5.0, 6.0},
        {7.0, 8.0, 9.0}
    };
    EXPECT_FALSE(is_jagged(uniform_2d));
}

TEST_F(JaggedArrayTest, DetectJagged3DArray) {
    EXPECT_TRUE(is_jagged_3d(jagged_3d));
    
    std::vector<std::vector<std::vector<double>>> uniform_3d = {
        {
            {1.0, 2.0},
            {3.0, 4.0}
        },
        {
            {5.0, 6.0},
            {7.0, 8.0}
        }
    };
    EXPECT_FALSE(is_jagged_3d(uniform_3d));
}

TEST_F(JaggedArrayTest, Normalize2DJaggedArray) {
    Chunk<std::vector<double>> chunker;
    auto normalized = chunker.handle_jagged_2d(jagged_2d);
    
    EXPECT_EQ(normalized.size(), expected_normalized_2d.size());
    for (size_t i = 0; i < normalized.size(); ++i) {
        EXPECT_EQ(normalized[i].size(), expected_normalized_2d[i].size());
        for (size_t j = 0; j < normalized[i].size(); ++j) {
            EXPECT_DOUBLE_EQ(normalized[i][j], expected_normalized_2d[i][j]);
        }
    }
}

TEST_F(JaggedArrayTest, Normalize3DJaggedArray) {
    Chunk<std::vector<std::vector<double>>> chunker;
    auto normalized = chunker.handle_jagged_3d(jagged_3d);
    
    EXPECT_EQ(normalized.size(), expected_normalized_3d.size());
    for (size_t i = 0; i < normalized.size(); ++i) {
        EXPECT_EQ(normalized[i].size(), expected_normalized_3d[i].size());
        for (size_t j = 0; j < normalized[i].size(); ++j) {
            EXPECT_EQ(normalized[i][j].size(), expected_normalized_3d[i][j].size());
            for (size_t k = 0; k < normalized[i][j].size(); ++k) {
                EXPECT_DOUBLE_EQ(normalized[i][j][k], expected_normalized_3d[i][j][k]);
            }
        }
    }
}

TEST_F(JaggedArrayTest, HandleEmptyArrays) {
    Chunk<std::vector<double>> chunker;
    
    std::vector<std::vector<double>> empty_2d;
    auto normalized_2d = chunker.handle_jagged_2d(empty_2d);
    EXPECT_TRUE(normalized_2d.empty());
    
    Chunk<std::vector<std::vector<double>>> chunker_3d;
    std::vector<std::vector<std::vector<double>>> empty_3d;
    auto normalized_3d = chunker_3d.handle_jagged_3d(empty_3d);
    EXPECT_TRUE(normalized_3d.empty());
}

TEST_F(JaggedArrayTest, ValidateJaggedArrays) {
    Chunk<std::vector<double>> chunker;
    
    // Should not throw for jagged arrays - they get normalized
    EXPECT_NO_THROW(chunker.validate_dimensions(jagged_2d));
    
    // Should not throw for empty inner arrays - they get normalized
    std::vector<std::vector<double>> jagged_with_empty = {
        {1.0, 2.0},
        {3.0, 4.0, 5.0},
        {}  // Empty inner array
    };
    EXPECT_NO_THROW(chunker.validate_dimensions(jagged_with_empty));

    // Should still throw for invalid input
    std::vector<std::vector<double>> invalid_data;
    EXPECT_THROW(chunker.validate_dimensions(invalid_data, 5), std::invalid_argument);
} 

#endif // CHUNK_PROCESSING_JAGGED_ARRAY_TEST_HPP 