#include "advanced_structures.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <random>

using namespace advanced_structures;

class ChunkSkipListTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1, 2, 3, 4, 5};
    }
    std::vector<int> test_data;
};

class ChunkBPlusTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_data = {1, 2, 3, 4, 5};
    }
    std::vector<int> test_data;
};

TEST_F(ChunkSkipListTest, BasicOperations) {
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

TEST_F(ChunkBPlusTreeTest, BasicOperations) {
    ChunkBPlusTree<int> tree;

    // Test insertion
    std::vector<int> values = {3, 7, 1, 5, 9, 2, 8, 4, 6};
    for (int val : values) {
        tree.insert(val);
    }

    // Test if values exist in tree
    for (int val : values) {
        EXPECT_TRUE(tree.search(val));
    }

    // Test non-existent values
    EXPECT_FALSE(tree.search(0));
    EXPECT_FALSE(tree.search(10));
}

TEST_F(ChunkSkipListTest, StressTest) {
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

TEST_F(ChunkSkipListTest, EmptyOperations) {
    ChunkSkipList<int> list;
    EXPECT_TRUE(list.search(1) == false);
    EXPECT_TRUE(list.search(0) == false);
}

TEST_F(ChunkBPlusTreeTest, EdgeCases) {
    ChunkBPlusTree<int> tree;
    tree.insert(1);
    tree.insert(2);

    // Verify insertions using find
    EXPECT_TRUE(tree.search(1));
    EXPECT_TRUE(tree.search(2));
    EXPECT_FALSE(tree.search(3));
}

TEST(ChunkDequeTest, BasicOperations) {
    ChunkDeque<int> deque;
    EXPECT_TRUE(deque.empty());

    deque.push_back(1);
    deque.push_front(0);
    EXPECT_EQ(deque.size(), 2);

    EXPECT_EQ(deque.pop_back(), 1);
    EXPECT_EQ(deque.pop_front(), 0);
    EXPECT_TRUE(deque.empty());
}

TEST(ChunkStackTest, BasicOperations) {
    ChunkStack<int> stack;
    EXPECT_TRUE(stack.empty());

    stack.push(1);
    stack.push(2);
    EXPECT_EQ(stack.size(), 2);

    EXPECT_EQ(stack.pop(), 2);
    EXPECT_EQ(stack.pop(), 1);
    EXPECT_TRUE(stack.empty());
}