#include <gtest/gtest.h>
#include "../data_structures.hpp"

TEST(CircularBufferTest, BasicOperations) {
    CircularBuffer<int> buffer(3);
    EXPECT_TRUE(buffer.empty());
    EXPECT_FALSE(buffer.full());
    
    buffer.push(1);
    EXPECT_EQ(buffer.size(), 1);
    
    buffer.push(2);
    buffer.push(3);
    EXPECT_TRUE(buffer.full());
    
    buffer.push(4);  // Should overwrite 1
    auto vec = buffer.to_vector();
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 2);
    EXPECT_EQ(vec[2], 4);
}

TEST(SlidingWindowTest, Average) {
    SlidingWindow<double> window(3);
    window.push(1.0);
    window.push(2.0);
    window.push(3.0);
    EXPECT_DOUBLE_EQ(window.average(), 2.0);
    
    window.push(4.0);
    EXPECT_DOUBLE_EQ(window.average(), 3.0);
}

TEST(ChunkListTest, Operations) {
    ChunkList<int> list;
    list.append_chunk({1, 2});
    list.append_chunk({3, 4});
    list.prepend_chunk({-1, 0});
    
    auto flat = list.flatten();
    EXPECT_EQ(flat.size(), 6);
    EXPECT_EQ(flat[0], -1);
    EXPECT_EQ(flat[5], 4);
}

TEST(PriorityQueueTest, Ordering) {
    PriorityQueue<int> pq;
    pq.push(3);
    pq.push(1);
    pq.push(4);
    
    EXPECT_EQ(pq.pop(), 4);
    EXPECT_EQ(pq.pop(), 3);
    EXPECT_EQ(pq.pop(), 1);
    EXPECT_TRUE(pq.empty());
} 