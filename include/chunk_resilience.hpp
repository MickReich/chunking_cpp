#pragma once

#include "chunk_common.hpp"
#include "chunk_errors.hpp"
#include <algorithm> // for std::min_element, std::max_element, std::sort, std::find_if
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional> // for std::greater
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h> // for getpagesize()
#include <unordered_map>
#include <vector>

namespace chunk_resilience {

/**
 * @brief Checkpoint data structure
 */
template <typename T>
struct CHUNK_EXPORT Checkpoint {
    size_t sequence_number;
    std::vector<std::vector<T>> chunks;
    std::chrono::system_clock::time_point timestamp;
    size_t memory_usage;
    bool is_corrupted;

    // Serialization
    void serialize(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            throw chunk_processing::ChunkingError("Failed to create checkpoint file: " + filename);
        }

        // Write metadata
        file.write(reinterpret_cast<const char*>(&sequence_number), sizeof(sequence_number));
        file.write(reinterpret_cast<const char*>(&memory_usage), sizeof(memory_usage));

        // Write chunks
        size_t num_chunks = chunks.size();
        file.write(reinterpret_cast<const char*>(&num_chunks), sizeof(num_chunks));

        for (const auto& chunk : chunks) {
            size_t chunk_size = chunk.size();
            file.write(reinterpret_cast<const char*>(&chunk_size), sizeof(chunk_size));
            file.write(reinterpret_cast<const char*>(chunk.data()), chunk_size * sizeof(T));
        }
    }

    // Deserialization
    static Checkpoint<T> deserialize(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw chunk_processing::ChunkingError("Failed to open checkpoint file: " + filename);
        }

        Checkpoint<T> checkpoint;

        try {
            // Read metadata
            file.read(reinterpret_cast<char*>(&checkpoint.sequence_number),
                      sizeof(checkpoint.sequence_number));
            file.read(reinterpret_cast<char*>(&checkpoint.memory_usage),
                      sizeof(checkpoint.memory_usage));

            // Read chunks
            size_t num_chunks;
            file.read(reinterpret_cast<char*>(&num_chunks), sizeof(num_chunks));

            checkpoint.chunks.resize(num_chunks);
            for (auto& chunk : checkpoint.chunks) {
                size_t chunk_size;
                file.read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));

                chunk.resize(chunk_size);
                file.read(reinterpret_cast<char*>(chunk.data()), chunk_size * sizeof(T));
            }

            checkpoint.timestamp = std::chrono::system_clock::now();
            checkpoint.is_corrupted = false;

        } catch (const std::exception& e) {
            checkpoint.is_corrupted = true;
            throw chunk_processing::ChunkingError("Checkpoint corruption detected: " +
                                                  std::string(e.what()));
        }

        return checkpoint;
    }
};

/**
 * @brief Resilient chunking system with error recovery
 */
template <typename T>
class CHUNK_EXPORT ResilientChunker {
private:
    std::string checkpoint_dir;
    size_t max_memory_usage;
    size_t checkpoint_interval;
    std::atomic<size_t> current_sequence;

    std::mutex mutex;
    std::condition_variable cv;

    std::unordered_map<size_t, Checkpoint<T>> checkpoint_history;
    size_t max_history_size;

    // Add memory tracking
    struct MemoryTracker {
        size_t current_usage = 0;
        std::mutex mutex;

        void add(size_t bytes) {
            std::lock_guard<std::mutex> lock(mutex);
            current_usage += bytes;
        }

        void subtract(size_t bytes) {
            std::lock_guard<std::mutex> lock(mutex);
            current_usage = (current_usage > bytes) ? current_usage - bytes : 0;
        }

        size_t get() const {
            return current_usage;
        }

        void reset() {
            std::lock_guard<std::mutex> lock(mutex);
            current_usage = 0;
        }
    };

    MemoryTracker memory_tracker;
    static constexpr size_t CHUNK_BUFFER_SIZE = 1000;
    static constexpr double MEMORY_THRESHOLD = 0.8; // 80% threshold

    // Smart chunk buffer with automatic memory tracking
    class ChunkBuffer {
        std::vector<std::vector<T>> chunks;
        MemoryTracker& tracker;
        const size_t max_size;

    public:
        ChunkBuffer(MemoryTracker& t, size_t max_sz) : tracker(t), max_size(max_sz) {
            chunks.reserve(max_sz);
        }

        void add_chunk(std::vector<T>&& chunk) {
            size_t chunk_memory = chunk.capacity() * sizeof(T);
            if (chunks.size() >= max_size ||
                (tracker.get() + chunk_memory) > max_size * sizeof(T)) {
                flush();
            }
            tracker.add(chunk_memory);
            chunks.push_back(std::move(chunk));
        }

        std::vector<std::vector<T>> flush() {
            std::vector<std::vector<T>> result;
            result.swap(chunks);
            tracker.reset();
            chunks.reserve(max_size);
            return result;
        }

        ~ChunkBuffer() {
            tracker.subtract(chunks.capacity() * sizeof(T));
        }
    };

    std::unique_ptr<ChunkBuffer> chunk_buffer;

    // Add current data storage
    std::vector<std::vector<T>> current_data;

    // Use RAII for mutex locking
    class ScopedLock {
        std::mutex& mutex_;
        bool locked_ = false;

    public:
        explicit ScopedLock(std::mutex& mutex, bool try_lock = false) : mutex_(mutex) {
            if (try_lock) {
                locked_ = mutex_.try_lock();
            } else {
                mutex_.lock();
                locked_ = true;
            }
        }
        ~ScopedLock() {
            if (locked_)
                mutex_.unlock();
        }
        bool locked() const {
            return locked_;
        }
    };

    /**
     * @brief Create a checkpoint of current state
     */
    void create_checkpoint(const std::vector<std::vector<T>>& chunks) {
        std::unique_lock<std::mutex> lock(mutex);
        if (chunks.empty())
            return;

        current_data = chunks; // Store current data
        size_t seq_num = current_sequence++;

        try {
            Checkpoint<T> checkpoint{seq_num, chunks, std::chrono::system_clock::now(),
                                     get_memory_usage(), false};

            std::string filename = checkpoint_dir + "/checkpoint_" +
                                   std::to_string(checkpoint.sequence_number) + ".bin";

            // Unlock while writing to file
            lock.unlock();
            checkpoint.serialize(filename);

            // Relock for history update
            lock.lock();
            checkpoint_history[checkpoint.sequence_number] = checkpoint;

            if (checkpoint_history.size() > max_history_size) {
                auto oldest =
                    std::min_element(checkpoint_history.begin(), checkpoint_history.end(),
                                     [](const auto& a, const auto& b) {
                                         return a.second.sequence_number < b.second.sequence_number;
                                     });
                std::filesystem::remove(checkpoint_dir + "/checkpoint_" +
                                        std::to_string(oldest->first) + ".bin");
                checkpoint_history.erase(oldest);
            }
        } catch (const std::exception& e) {
            throw chunk_processing::ChunkingError("Failed to create checkpoint: " +
                                                  std::string(e.what()));
        }
    }

    /**
     * @brief Get current memory usage
     */
    size_t get_memory_usage() const {
#ifdef __linux__
        std::ifstream stat("/proc/self/statm");
        size_t resident;
        stat >> resident; // Skip first value
        stat >> resident; // Read resident set size
#ifdef _SC_PAGESIZE
        return resident * sysconf(_SC_PAGESIZE);
#else
        return resident * 4096; // Default page size
#endif
#else
        return 0; // Placeholder for non-Linux systems
#endif
    }

    /**
     * @brief Verify checkpoint integrity
     */
    bool verify_checkpoint(const Checkpoint<T>& checkpoint) {
        if (checkpoint.is_corrupted) {
            return false;
        }

        if (checkpoint.memory_usage > max_memory_usage) {
            return false;
        }

        // Additional validation
        if (checkpoint.chunks.empty()) {
            return false;
        }

        // Verify each chunk
        for (const auto& chunk : checkpoint.chunks) {
            if (chunk.empty()) {
                return false;
            }
        }

        return true;
    }

    void initialize_checkpoint_dir() {
        try {
            std::filesystem::create_directories(checkpoint_dir);
        } catch (const std::exception& e) {
            throw chunk_processing::ResilienceError("Failed to create checkpoint directory: " +
                                                    std::string(e.what()));
        }
    }

public:
    ResilientChunker(const std::string& checkpoint_directory = "./checkpoints",
                     size_t max_mem_usage = 1024 * 1024 * 1024, // 1GB
                     size_t checkpoint_freq = 1000,             // chunks
                     size_t history_size = 5)                   // checkpoints
        : checkpoint_dir(checkpoint_directory), max_memory_usage(max_mem_usage),
          checkpoint_interval(checkpoint_freq), current_sequence(0),
          max_history_size(history_size) {
        // Create checkpoint directory
        std::filesystem::create_directories(checkpoint_dir);
    }

    /**
     * @brief Process data with checkpointing and error recovery
     */
    std::vector<std::vector<T>> process(const std::vector<T>& data) {
        if (data.empty()) {
            throw chunk_processing::ResilienceError("Cannot process empty data");
        }

        try {
            initialize_checkpoint_dir();
            current_data.clear(); // Clear any old data

            // Create initial empty checkpoint
            std::vector<std::vector<T>> initial_chunk;
            initial_chunk.push_back(std::vector<T>{data[0]}); // Start with first element
            create_checkpoint(initial_chunk);

            size_t processed = 0;

            chunk_buffer = std::make_unique<ChunkBuffer>(memory_tracker, CHUNK_BUFFER_SIZE);

            while (processed < data.size()) {
                if (get_memory_usage() > max_memory_usage * MEMORY_THRESHOLD) {
                    auto flushed = chunk_buffer->flush();
                    current_data.insert(current_data.end(),
                                        std::make_move_iterator(flushed.begin()),
                                        std::make_move_iterator(flushed.end()));
                    create_checkpoint(current_data);
                }

                size_t chunk_size = std::min(checkpoint_interval, data.size() - processed);
                std::vector<T> current_chunk(data.begin() + processed,
                                             data.begin() + processed + chunk_size);

                chunk_buffer->add_chunk(std::move(current_chunk));
                processed += chunk_size;
            }

            // Final flush
            auto final_chunks = chunk_buffer->flush();
            current_data.insert(current_data.end(), std::make_move_iterator(final_chunks.begin()),
                                std::make_move_iterator(final_chunks.end()));

            create_checkpoint(current_data);
            return current_data;

        } catch (const std::exception& e) {
            if (chunk_buffer) {
                chunk_buffer->flush();
            }
            memory_tracker.reset();
            checkpoint_history.clear();
            current_data.clear();

            throw chunk_processing::ResilienceError("Processing failed: " + std::string(e.what()));
        }
    }

    /**
     * @brief Save current state
     */
    void save_checkpoint() {
        std::unique_lock<std::mutex> lock(mutex);

        // First check current data
        if (!current_data.empty()) {
            auto data_copy = current_data; // Make a copy while locked
            lock.unlock();
            create_checkpoint(data_copy);
            return;
        }

        // Then check checkpoint history
        if (!checkpoint_history.empty()) {
            auto latest =
                std::max_element(checkpoint_history.begin(), checkpoint_history.end(),
                                 [](const auto& a, const auto& b) {
                                     return a.second.sequence_number < b.second.sequence_number;
                                 });

            if (verify_checkpoint(latest->second)) {
                std::string filename = checkpoint_dir + "/latest_checkpoint.bin";
                auto checkpoint_copy = latest->second; // Make a copy while locked
                lock.unlock();
                checkpoint_copy.serialize(filename);
                return;
            }
        }

        // If we get here, we have no valid data to checkpoint
        throw chunk_processing::ChunkingError(
            "No data available for checkpoint creation. Process some data first.");
    }

    /**
     * @brief Restore from latest valid checkpoint
     */
    std::vector<std::vector<T>> restore_from_checkpoint() {
        ScopedLock lock(mutex, true); // Try lock to avoid deadlock
        if (!lock.locked()) {
            throw chunk_processing::ChunkingError("Failed to acquire lock for restore");
        }

        if (checkpoint_history.empty()) {
            throw chunk_processing::ChunkingError("No checkpoints available");
        }

        std::vector<size_t> keys;
        keys.reserve(checkpoint_history.size());
        for (const auto& pair : checkpoint_history) {
            keys.push_back(pair.first);
        }
        std::sort(keys.begin(), keys.end(), std::greater<size_t>());

        for (auto key : keys) {
            if (verify_checkpoint(checkpoint_history[key])) {
                return checkpoint_history[key].chunks;
            }
        }

        throw chunk_processing::ChunkingError("No valid checkpoint found for recovery");
    }

    /**
     * @brief Handle memory exhaustion
     */
    void handle_memory_exhaustion() {
        // Clear all buffers
        if (chunk_buffer) {
            chunk_buffer->flush();
        }

        // Clear checkpoint history
        checkpoint_history.clear();

        // Reset memory tracking
        memory_tracker.reset();

        // Force cleanup
        std::vector<std::vector<T>>().swap(checkpoint_history[current_sequence].chunks);

        // Wait for memory to be freed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (get_memory_usage() > max_memory_usage) {
            // Last resort: clear everything
            checkpoint_history.clear();
            current_sequence.store(0);

            if (get_memory_usage() > max_memory_usage) {
                throw chunk_processing::ChunkingError("Memory exhaustion: Unable to recover");
            }
        }
    }

    /**
     * @brief Handle data corruption
     */
    void handle_corruption() {
        std::cerr << "Data corruption detected at sequence " << current_sequence << std::endl;

        if (checkpoint_history.count(current_sequence)) {
            checkpoint_history[current_sequence].is_corrupted = true;
        }

        // Attempt to restore from last valid checkpoint
        auto valid_checkpoint = restore_from_checkpoint();

        // Find highest sequence number
        size_t max_seq = 0;
        for (const auto& [seq, _] : checkpoint_history) {
            max_seq = std::max(max_seq, seq);
        }
        current_sequence.store(max_seq);
    }

    // Add getters
    size_t get_max_memory_usage() const {
        return max_memory_usage;
    }
    size_t get_checkpoint_interval() const {
        return checkpoint_interval;
    }
    size_t get_max_history_size() const {
        return max_history_size;
    }
    size_t get_current_sequence() const {
        return current_sequence.load();
    }
    const std::string& get_checkpoint_dir() const {
        return checkpoint_dir;
    }

    // Add setters
    void set_max_memory_usage(size_t usage) {
        max_memory_usage = usage;
    }
    void set_checkpoint_interval(size_t interval) {
        checkpoint_interval = interval;
    }
    void set_max_history_size(size_t size) {
        max_history_size = size;
    }
};

} // namespace chunk_resilience