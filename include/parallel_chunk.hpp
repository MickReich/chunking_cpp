#ifndef PARALLEL_CHUNK_HPP
#define PARALLEL_CHUNK_HPP

#include "chunk.hpp"
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>
#include <numeric>

namespace parallel_chunk {

/**
 * @brief Parallel chunk processor for concurrent operations
 * @tparam T The type of elements to process
 */
template <typename T>
class ParallelChunkProcessor {
public:
    using ChunkOperation = std::function<void(std::vector<T>&)>;

    /**
     * @brief Process chunks in parallel
     * @param chunks Vector of chunks to process
     * @param operation Operation to apply to each chunk
     * @param num_threads Number of threads to use (default: hardware concurrency)
     */
    static void process_chunks(std::vector<std::vector<T>>& chunks,
                               const std::function<void(std::vector<T>&)>& operation) {
        if (chunks.empty()) return;

        struct SharedState {
            std::mutex mutex;
            std::exception_ptr exception{nullptr};
            bool has_error{false};
            bool should_stop{false};
            std::vector<std::vector<T>>* chunks_ptr{nullptr};
            const std::function<void(std::vector<T>&)>* operation_ptr{nullptr};

            // Constructor to properly initialize the state
            SharedState(std::vector<std::vector<T>>& chunks_ref,
                       const std::function<void(std::vector<T>&)>& operation_ref)
                : chunks_ptr(&chunks_ref)
                , operation_ptr(&operation_ref) {}
        };

        auto shared_state = std::make_shared<SharedState>(chunks, operation);

        std::vector<std::thread> threads;
        threads.reserve(chunks.size());

        // Create thread-safe worker function
        auto worker = [](size_t index, std::shared_ptr<SharedState> state) {
            if (state->should_stop) return;
            
            try {
                if (index < state->chunks_ptr->size()) {
                    (*state->operation_ptr)((*state->chunks_ptr)[index]);
                }
            } catch (...) {
                std::lock_guard<std::mutex> lock(state->mutex);
                if (!state->has_error) {
                    state->exception = std::current_exception();
                    state->has_error = true;
                    state->should_stop = true;
                }
            }
        };

        // Launch threads using indices
        for (size_t i = 0; i < chunks.size(); ++i) {
            if (shared_state->should_stop) break;
            threads.emplace_back(worker, i, shared_state);
        }

        // Join all threads
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Check for exceptions after all threads complete
        if (shared_state->has_error && shared_state->exception) {
            std::rethrow_exception(shared_state->exception);
        }
    }

    /**
     * @brief Map operation over chunks in parallel
     * @param chunks Input chunks
     * @param operation Mapping operation
     * @return Transformed chunks
     */
    template <typename U>
    static std::vector<std::vector<U>> map(const std::vector<std::vector<T>>& chunks,
                                           std::function<U(const T&)> operation) {
        std::vector<std::vector<U>> result(chunks.size());
        std::vector<std::future<void>> futures;

        for (size_t i = 0; i < chunks.size(); ++i) {
            futures.push_back(std::async(std::launch::async, [&, i]() {
                result[i].reserve(chunks[i].size());
                std::transform(chunks[i].begin(), chunks[i].end(), std::back_inserter(result[i]),
                               operation);
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }

        return result;
    }

    /**
     * @brief Reduce chunks in parallel
     * @param chunks Input chunks
     * @param operation Reduction operation
     * @param initial Initial value
     * @return Reduced value
     */
    static T reduce(const std::vector<std::vector<T>>& chunks,
                    std::function<T(const T&, const T&)> operation, T initial) {
        std::vector<std::future<T>> futures;

        for (const auto& chunk : chunks) {
            futures.push_back(std::async(std::launch::async, [&]() {
                return std::accumulate(chunk.begin(), chunk.end(), T(), operation);
            }));
        }

        T result = initial;
        for (auto& future : futures) {
            result = operation(result, future.get());
        }

        return result;
    }
};

} // namespace parallel_chunk

#endif // PARALLEL_CHUNK_HPP