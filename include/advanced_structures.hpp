#ifndef ADVANCED_STRUCTURES_HPP
#define ADVANCED_STRUCTURES_HPP

#include <algorithm>
#include <deque>
#include <memory>
#include <stack>
#include <stdexcept>
#include <vector>

namespace advanced_structures {

/**
 * @brief A skip list implementation for efficient chunk searching
 * @tparam T The type of elements stored in the skip list
 */
template <typename T>
class ChunkSkipList {
private:
    struct Node {
        T value;
        std::vector<std::shared_ptr<Node>> forward;
        explicit Node(T val, int level) : value(val), forward(level) {}
    };

    std::shared_ptr<Node> head;
    int max_level;
    float p;
    int current_level;

    int random_level() {
        int lvl = 1;
        while ((static_cast<float>(rand()) / RAND_MAX) < p && lvl < max_level) {
            lvl++;
        }
        return lvl;
    }

public:
    ChunkSkipList(int max_lvl = 16, float prob = 0.5)
        : max_level(max_lvl), p(prob), current_level(1) {
        head = std::make_shared<Node>(T(), max_level);
    }

    void insert(const T& value) {
        std::vector<std::shared_ptr<Node>> update(max_level);
        auto current = head;

        for (int i = current_level - 1; i >= 0; i--) {
            while (current->forward[i] && current->forward[i]->value < value) {
                current = current->forward[i];
            }
            update[i] = current;
        }

        int new_level = random_level();
        if (new_level > current_level) {
            for (int i = current_level; i < new_level; i++) {
                update[i] = head;
            }
            current_level = new_level;
        }

        auto new_node = std::make_shared<Node>(value, new_level);
        for (int i = 0; i < new_level; i++) {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;
        }
    }

    bool search(const T& value) const {
        auto current = head;
        for (int i = current_level - 1; i >= 0; i--) {
            while (current->forward[i] && current->forward[i]->value < value) {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        return current && current->value == value;
    }
};

/**
 * @brief A B+ tree implementation for chunk indexing
 * @tparam T The type of elements stored in the tree
 */
template <typename T>
class ChunkBPlusTree {
    static constexpr int ORDER = 4;

    struct Node {
        bool is_leaf;
        std::vector<T> keys;
        std::vector<std::shared_ptr<Node>> children;
        std::shared_ptr<Node> next;

        Node(bool leaf = true) : is_leaf(leaf) {}
    };

    std::shared_ptr<Node> root;

public:
    ChunkBPlusTree() : root(std::make_shared<Node>()) {}

    void insert(const T& key) {
        if (root->keys.empty()) {
            root->keys.push_back(key);
            return;
        }

        if (root->keys.size() == ORDER - 1) {
            auto new_root = std::make_shared<Node>(false);
            new_root->children.push_back(root);
            split_child(new_root, 0);
            root = new_root;
        }
        insert_non_full(root, key);
    }

    bool search(const T& key) const {
        if (root == nullptr)
            return false;

        return search_node(root, key);
    }

private:
    void split_child(std::shared_ptr<Node> parent, int index) {
        auto child = parent->children[index];
        auto new_child = std::make_shared<Node>(child->is_leaf);

        parent->keys.insert(parent->keys.begin() + index, child->keys[ORDER / 2 - 1]);

        parent->children.insert(parent->children.begin() + index + 1, new_child);

        new_child->keys.assign(child->keys.begin() + ORDER / 2, child->keys.end());

        child->keys.resize(ORDER / 2 - 1);

        if (!child->is_leaf) {
            new_child->children.assign(child->children.begin() + ORDER / 2, child->children.end());
            child->children.resize(ORDER / 2);
        }
    }

    void insert_non_full(std::shared_ptr<Node> node, const T& key) {
        int i = node->keys.size() - 1;

        if (node->is_leaf) {
            node->keys.push_back(T());
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;
        } else {
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;

            if (node->children[i]->keys.size() == ORDER - 1) {
                split_child(node, i);
                if (key > node->keys[i]) {
                    i++;
                }
            }
            insert_non_full(node->children[i], key);
        }
    }

    bool search_node(const std::shared_ptr<Node>& node, const T& key) const {
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {
            i++;
        }

        if (i < node->keys.size() && key == node->keys[i]) {
            return true;
        }

        if (node->is_leaf) {
            return false;
        }

        return search_node(node->children[i], key);
    }
};

/**
 * @brief A deque-based chunk structure for double-ended operations
 * @tparam T The type of elements stored in the chunk deque
 */
template <typename T>
class ChunkDeque {
private:
    std::deque<T> data_;

public:
    void push_back(const T& value) {
        data_.push_back(value);
    }

    void push_front(const T& value) {
        data_.push_front(value);
    }

    T pop_back() {
        T value = data_.back();
        data_.pop_back();
        return value;
    }

    T pop_front() {
        T value = data_.front();
        data_.pop_front();
        return value;
    }

    size_t size() const {
        return data_.size();
    }

    bool empty() const {
        return data_.empty();
    }
};

/**
 * @brief A stack-based chunk structure for LIFO operations
 * @tparam T The type of elements stored in the chunk stack
 */
template <typename T>
class ChunkStack {
private:
    std::stack<T> data_;

public:
    void push(const T& value) {
        data_.push(value);
    }

    T pop() {
        T value = data_.top();
        data_.pop();
        return value;
    }

    size_t size() const {
        return data_.size();
    }

    bool empty() const {
        return data_.empty();
    }
};

} // namespace advanced_structures

#endif // ADVANCED_STRUCTURES_HPP