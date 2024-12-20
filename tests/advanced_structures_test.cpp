#include "advanced_structures.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <random>

using namespace advanced_structures;

TEST(ChunkSkipListTest, BasicOperations) {
    ChunkSkipList<int> skip_list;

    // Test insertion
    skip_list.insert(5);
    skip_list.insert(10);
    skip_list.insert(7);

    // Test search
    EXPECT_TRUE(skip_list.search(5));
    EXPECT_TRUE(skip_list.search(7));
    EXPECT_TRUE(skip_list.search(10));
    EXPECT_FALSE(skip_list.search(6));

    // Test with random data
    std::vector<int> numbers(100);
    std::iota(numbers.begin(), numbers.end(), 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(numbers.begin(), numbers.end(), gen);

    ChunkSkipList<int> large_list;
    for (int num : numbers) {
        large_list.insert(num);
    }

    // Verify all numbers can be found
    for (int num : numbers) {
        EXPECT_TRUE(large_list.search(num));
    }
}

TEST(ChunkBPlusTreeTest, BasicOperations) {
    ChunkBPlusTree<int> tree;

    // Test insertion
    std::vector<int> values = {3, 7, 1, 5, 9, 2, 8, 4, 6};
    for (int val : values) {
        tree.insert(val);
    }

    // Sort values for comparison
    std::sort(values.begin(), values.end());

    // Test traversal (implement if needed)
    // auto result = tree.traverse();
    // EXPECT_EQ(result, values);
}

TEST(ChunkSkipListTest, StressTest) {
    ChunkSkipList<int> skip_list;
    const int NUM_ELEMENTS = 10000;

    // Insert many elements
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        skip_list.insert(i);
    }

    // Verify all elements can be found
    for (int i = 0; i < NUM_ELEMENTS; i++) {
        EXPECT_TRUE(skip_list.search(i));
    }

    // Verify non-existent elements are not found
    for (int i = NUM_ELEMENTS; i < NUM_ELEMENTS + 100; i++) {
        EXPECT_FALSE(skip_list.search(i));
    }
}