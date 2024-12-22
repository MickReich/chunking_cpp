#include "advanced_structures.hpp"
#include "gtest/gtest.h"
#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

using namespace advanced_structures;

// Helper function to print a vector
template <typename T>
void printVector(const std::vector<T>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

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

TEST(ChunkTreapTest, BasicOperations) {
    ChunkTreap<int> treap;
    EXPECT_FALSE(treap.search(5));

    treap.insert(5);
    treap.insert(3);
    treap.insert(8);

    EXPECT_TRUE(treap.search(5));
    EXPECT_TRUE(treap.search(3));
    EXPECT_TRUE(treap.search(8));
    EXPECT_FALSE(treap.search(10));
}

TEST(AdaptiveChunkTreeTest, BasicOperations) {
    AdaptiveChunkTree<int> tree;
    std::vector<int> data = {1, 2, 3, 10, 11, 12, 20, 21, 22};
    auto chunks = tree.chunk(data);

    // Test that chunks were created
    EXPECT_GT(chunks.size(), 0);

    // Test that all elements are preserved
    size_t total_elements = 0;
    for (const auto& chunk : chunks) {
        total_elements += chunk.size();
    }
    EXPECT_EQ(total_elements, data.size());
}

TEST(AdaptiveChunkTreeTest, SpecializedTypes) {
    AdaptiveChunkTree<uint8_t> binary_tree;
    std::vector<uint8_t> binary_data = {0x00, 0x00, 0xFF, 0xFF, 0xAA, 0x55};
    auto binary_chunks = binary_tree.chunk(binary_data);
    EXPECT_GT(binary_chunks.size(), 0);

    AdaptiveChunkTree<char> char_tree;
    std::vector<char> char_data = {'a', 'a', 'b', 'c', 'c', 'd'};
    auto char_chunks = char_tree.chunk(char_data);
    EXPECT_GT(char_chunks.size(), 0);
}

TEST(AdaptiveChunkTreeTest, EdgeCases) {
    AdaptiveChunkTree<int> tree;

    std::vector<int> empty_data;
    auto empty_chunks = tree.chunk(empty_data);
    EXPECT_TRUE(empty_chunks.empty());

    std::vector<int> single_data = {1};
    auto single_chunks = tree.chunk(single_data);
    EXPECT_EQ(single_chunks.size(), 1);
    EXPECT_EQ(single_chunks[0].size(), 1);

    std::vector<int> uniform_data(10, 5);
    auto uniform_chunks = tree.chunk(uniform_data);
    EXPECT_GT(uniform_chunks.size(), 0);

    std::vector<int> alternating_data = {1, 10, 1, 10, 1, 10};
    auto alternating_chunks = tree.chunk(alternating_data);
    EXPECT_GT(alternating_chunks.size(), 1);
}

TEST(SemanticBoundariesChunkTest, BasicOperations) {
    SemanticBoundariesChunk<std::string> chunk;
    std::vector<std::string> data = {"This", "is", "a", "test", "sentence"};
    auto result = chunk.chunk(data);
    EXPECT_GT(result.size(), 0);
}

TEST(FractalPatternsChunkTest, BasicOperations) {
    FractalPatternsChunk<int> chunk;
    std::vector<int> data = {1, 2, 3, 4, 5, 1, 2, 3};
    auto result = chunk.chunk(data);
    EXPECT_GT(result.size(), 0);
}

TEST(BloomFilterChunkTest, BasicOperations) {
    BloomFilterChunk<int> chunk;
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto result = chunk.chunk(data);
    EXPECT_GT(result.size(), 0);
}

TEST(GraphBasedChunkTest, BasicOperations) {
    GraphBasedChunk<int> chunk;
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto result = chunk.chunk(data);
    EXPECT_GT(result.size(), 0);
}