#ifndef PARALLEL_CHUNK_HPP
#define PARALLEL_CHUNK_HPP

#include "chunk.hpp"
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

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
        std::mutex exception_mutex;
        std::exception_ptr exception_ptr = nullptr;
        std::vector<std::thread> threads;

        for (auto& chunk : chunks) {
            threads.emplace_back([&chunk, &operation, &exception_mutex, &exception_ptr]() {
                try {
                    operation(chunk);
                } catch (...) {
                    std::lock_guard<std::mutex> lock(exception_mutex);
                    if (!exception_ptr) {
                        exception_ptr = std::current_exception();
                    }
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        if (exception_ptr) {
            std::rethrow_exception(exception_ptr);
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