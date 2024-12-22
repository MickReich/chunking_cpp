#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace chunk_resilience {

/**
 * @brief Exception class for resilient chunking errors
 */
class ChunkingError : public std::runtime_error {
public:
    explicit ChunkingError(const std::string& message) 
        : std::runtime_error(message) {}
};

/**
 * @brief Checkpoint data structure
 */
template<typename T>
struct Checkpoint {
    size_t sequence_number;
    std::vector<std::vector<T>> chunks;
    std::chrono::system_clock::time_point timestamp;
    size_t memory_usage;
    bool is_corrupted;

    // Serialization
    void serialize(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            throw ChunkingError("Failed to create checkpoint file: " + filename);
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
            file.write(reinterpret_cast<const char*>(chunk.data()), 
                      chunk_size * sizeof(T));
        }
    }

    // Deserialization
    static Checkpoint<T> deserialize(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw ChunkingError("Failed to open checkpoint file: " + filename);
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
                file.read(reinterpret_cast<char*>(chunk.data()), 
                         chunk_size * sizeof(T));
            }

            checkpoint.timestamp = std::chrono::system_clock::now();
            checkpoint.is_corrupted = false;

        } catch (const std::exception& e) {
            checkpoint.is_corrupted = true;
            throw ChunkingError("Checkpoint corruption detected: " + std::string(e.what()));
        }

        return checkpoint;
    }
};

/**
 * @brief Resilient chunking system with error recovery
 */
template<typename T>
class ResilientChunker {
private:
    std::string checkpoint_dir;
    size_t max_memory_usage;
    size_t checkpoint_interval;
    std::atomic<size_t> current_sequence;
    
    std::mutex mutex;
    std::condition_variable cv;
    
    std::unordered_map<size_t, Checkpoint<T>> checkpoint_history;
    size_t max_history_size;

    /**
     * @brief Create a checkpoint of current state
     */
    void create_checkpoint(const std::vector<std::vector<T>>& chunks) {
        std::lock_guard<std::mutex> lock(mutex);
        
        Checkpoint<T> checkpoint{
            current_sequence++,
            chunks,
            std::chrono::system_clock::now(),
            get_memory_usage(),
            false
        };

        // Save checkpoint to file
        std::string filename = checkpoint_dir + "/checkpoint_" + 
                             std::to_string(checkpoint.sequence_number) + ".bin";
        checkpoint.serialize(filename);

        // Add to history
        checkpoint_history[checkpoint.sequence_number] = checkpoint;

        // Maintain history size
        if (checkpoint_history.size() > max_history_size) {
            auto oldest = std::min_element(
                checkpoint_history.begin(), 
                checkpoint_history.end(),
                [](const auto& a, const auto& b) {
                    return a.second.sequence_number < b.second.sequence_number;
                }
            );
            
            std::filesystem::remove(checkpoint_dir + "/checkpoint_" + 
                                  std::to_string(oldest->first) + ".bin");
            checkpoint_history.erase(oldest);
        }
    }

    /**
     * @brief Get current memory usage
     */
    size_t get_memory_usage() const {
        std::ifstream stat("/proc/self/statm");
        size_t resident;
        stat >> resident; // Skip first value
        stat >> resident; // Read resident set size
        return resident * getpagesize();
    }

    /**
     * @brief Verify checkpoint integrity
     */
    bool verify_checkpoint(const Checkpoint<T>& checkpoint) {
        return !checkpoint.is_corrupted && 
               checkpoint.memory_usage <= max_memory_usage;
    }

public:
    ResilientChunker(const std::string& checkpoint_directory = "./checkpoints",
                     size_t max_mem_usage = 1024 * 1024 * 1024,  // 1GB
                     size_t checkpoint_freq = 1000,               // chunks
                     size_t history_size = 5)                     // checkpoints
        : checkpoint_dir(checkpoint_directory)
        , max_memory_usage(max_mem_usage)
        , checkpoint_interval(checkpoint_freq)
        , current_sequence(0)
        , max_history_size(history_size) {
        
        // Create checkpoint directory
        std::filesystem::create_directories(checkpoint_dir);
    }

    /**
     * @brief Process data with checkpointing and error recovery
     */
    std::vector<std::vector<T>> process(const std::vector<T>& data) {
        std::vector<std::vector<T>> result;
        size_t processed = 0;

        try {
            while (processed < data.size()) {
                // Check memory usage
                if (get_memory_usage() > max_memory_usage) {
                    handle_memory_exhaustion();
                }

                // Process next chunk
                size_t chunk_size = std::min(checkpoint_interval, 
                                           data.size() - processed);
                std::vector<T> current_chunk(data.begin() + processed,
                                           data.begin() + processed + chunk_size);
                result.push_back(current_chunk);
                processed += chunk_size;

                // Create checkpoint if needed
                if (processed % checkpoint_interval == 0) {
                    create_checkpoint(result);
                }
            }

        } catch (const std::exception& e) {
            // Handle error and attempt recovery
            handle_corruption();
            result = restore_from_checkpoint();
        }

        return result;
    }

    /**
     * @brief Save current state
     */
    void save_checkpoint() {
        if (!checkpoint_history.empty()) {
            auto latest = std::max_element(
                checkpoint_history.begin(),
                checkpoint_history.end(),
                [](const auto& a, const auto& b) {
                    return a.second.sequence_number < b.second.sequence_number;
                }
            );
            latest->second.serialize(checkpoint_dir + "/latest_checkpoint.bin");
        }
    }

    /**
     * @brief Restore from latest valid checkpoint
     */
    std::vector<std::vector<T>> restore_from_checkpoint() {
        std::lock_guard<std::mutex> lock(mutex);

        // Find latest valid checkpoint
        auto latest_valid = std::find_if(
            checkpoint_history.rbegin(),
            checkpoint_history.rend(),
            [this](const auto& checkpoint) {
                return verify_checkpoint(checkpoint.second);
            }
        );

        if (latest_valid == checkpoint_history.rend()) {
            throw ChunkingError("No valid checkpoint found for recovery");
        }

        return latest_valid->second.chunks;
    }

    /**
     * @brief Handle memory exhaustion
     */
    void handle_memory_exhaustion() {
        // Free up memory by clearing old checkpoints
        checkpoint_history.clear();
        
        // Force garbage collection
        std::vector<std::vector<T>>().swap(checkpoint_history[current_sequence].chunks);
        
        // Wait for memory to be freed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        if (get_memory_usage() > max_memory_usage) {
            throw ChunkingError("Memory exhaustion: Unable to recover");
        }
    }

    /**
     * @brief Handle data corruption
     */
    void handle_corruption() {
        // Log corruption event
        std::cerr << "Data corruption detected at sequence " 
                  << current_sequence << std::endl;
        
        // Mark current checkpoint as corrupted
        if (checkpoint_history.count(current_sequence)) {
            checkpoint_history[current_sequence].is_corrupted = true;
        }
        
        // Attempt to restore from last valid checkpoint
        auto valid_checkpoint = restore_from_checkpoint();
        
        // Reset sequence number
        current_sequence = checkpoint_history.rbegin()->first;
    }
};

} // namespace chunk_resilience