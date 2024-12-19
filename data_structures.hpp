#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP

#include <vector>
#include <deque>
#include <stdexcept>
#include <memory>
#include <functional>

// Circular Buffer - useful for sliding window operations
template<typename T>
class CircularBuffer {
private:
    std::vector<T> buffer_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
    size_t capacity_;

public:
    explicit CircularBuffer(size_t capacity) : 
        buffer_(capacity),
        capacity_(capacity) {
        if (capacity == 0) throw std::invalid_argument("Capacity must be positive");
    }

    void push(const T& item) {
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % capacity_;
        if (size_ < capacity_) {
            ++size_;
        } else {
            head_ = (head_ + 1) % capacity_;
        }
    }

    T pop() {
        if (empty()) throw std::runtime_error("Buffer is empty");
        T item = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        --size_;
        return item;
    }

    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == capacity_; }
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }

    std::vector<T> to_vector() const {
        std::vector<T> result;
        result.reserve(size_);
        size_t current = head_;
        for (size_t i = 0; i < size_; ++i) {
            result.push_back(buffer_[current]);
            current = (current + 1) % capacity_;
        }
        return result;
    }
};

// Priority Queue with custom comparator - useful for chunk prioritization
template<typename T, typename Compare = std::less<T>>
class PriorityQueue {
private:
    std::vector<T> heap_;
    Compare comp_;

    void heapify_up(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (comp_(heap_[parent], heap_[index])) {
                std::swap(heap_[parent], heap_[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void heapify_down(size_t index) {
        while (true) {
            size_t largest = index;
            size_t left = 2 * index + 1;
            size_t right = 2 * index + 2;

            if (left < heap_.size() && comp_(heap_[largest], heap_[left])) {
                largest = left;
            }
            if (right < heap_.size() && comp_(heap_[largest], heap_[right])) {
                largest = right;
            }

            if (largest == index) {
                break;
            }

            std::swap(heap_[index], heap_[largest]);
            index = largest;
        }
    }

public:
    void push(const T& item) {
        heap_.push_back(item);
        heapify_up(heap_.size() - 1);
    }

    T pop() {
        if (empty()) throw std::runtime_error("Queue is empty");
        T result = heap_[0];
        heap_[0] = heap_.back();
        heap_.pop_back();
        if (!empty()) {
            heapify_down(0);
        }
        return result;
    }

    bool empty() const { return heap_.empty(); }
    size_t size() const { return heap_.size(); }
};

// Sliding Window - useful for moving average calculations
template<typename T>
class SlidingWindow {
private:
    std::deque<T> window_;
    size_t max_size_;
    T sum_ = T();

public:
    explicit SlidingWindow(size_t size) : max_size_(size) {
        if (size == 0) throw std::invalid_argument("Window size must be positive");
    }

    void push(const T& value) {
        window_.push_back(value);
        sum_ += value;

        if (window_.size() > max_size_) {
            sum_ -= window_.front();
            window_.pop_front();
        }
    }

    T average() const {
        if (window_.empty()) throw std::runtime_error("Window is empty");
        return sum_ / static_cast<T>(window_.size());
    }

    const std::deque<T>& window() const { return window_; }
    size_t size() const { return window_.size(); }
    bool empty() const { return window_.empty(); }
    T sum() const { return sum_; }
};

// Chunk Node for building chunk-based data structures
template<typename T>
struct ChunkNode {
    std::vector<T> data;
    std::shared_ptr<ChunkNode<T>> next;
    std::shared_ptr<ChunkNode<T>> prev;

    explicit ChunkNode(const std::vector<T>& chunk_data) : data(chunk_data) {}
};

// Doubly Linked Chunk List - useful for chunk manipulation
template<typename T>
class ChunkList {
private:
    std::shared_ptr<ChunkNode<T>> head_;
    std::shared_ptr<ChunkNode<T>> tail_;
    size_t size_ = 0;

public:
    void append_chunk(const std::vector<T>& chunk_data) {
        auto new_node = std::make_shared<ChunkNode<T>>(chunk_data);
        if (!head_) {
            head_ = tail_ = new_node;
        } else {
            new_node->prev = tail_;
            tail_->next = new_node;
            tail_ = new_node;
        }
        ++size_;
    }

    void prepend_chunk(const std::vector<T>& chunk_data) {
        auto new_node = std::make_shared<ChunkNode<T>>(chunk_data);
        if (!head_) {
            head_ = tail_ = new_node;
        } else {
            new_node->next = head_;
            head_->prev = new_node;
            head_ = new_node;
        }
        ++size_;
    }

    std::vector<T> flatten() const {
        std::vector<T> result;
        auto current = head_;
        while (current) {
            result.insert(result.end(), current->data.begin(), current->data.end());
            current = current->next;
        }
        return result;
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void clear() {
        head_ = tail_ = nullptr;
        size_ = 0;
    }
};

#endif // DATA_STRUCTURES_HPP 