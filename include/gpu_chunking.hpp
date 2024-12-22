#pragma once

#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <memory>
#include <stdexcept>
#include <vector>

namespace gpu_chunking {

// CUDA error checking helper
#define CUDA_CHECK(call)                                                                           \
    do {                                                                                           \
        cudaError_t error = call;                                                                  \
        if (error != cudaSuccess) {                                                                \
            throw std::runtime_error(std::string("CUDA error: ") + cudaGetErrorString(error));     \
        }                                                                                          \
    } while (0)

// Device code for parallel chunking
__global__ void chunk_kernel(const int* data, int* chunk_boundaries, int data_size, int window_size,
                             float threshold) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= data_size - window_size)
        return;

    // Compute local statistics for the window
    float window_sum = 0.0f;
    float window_max = data[idx];
    float window_min = data[idx];

    for (int i = 0; i < window_size; ++i) {
        int current = data[idx + i];
        window_sum += current;
        window_max = max(window_max, (float)current);
        window_min = min(window_min, (float)current);
    }

    float window_mean = window_sum / window_size;
    float variance = 0.0f;

    // Compute variance
    for (int i = 0; i < window_size; ++i) {
        float diff = data[idx + i] - window_mean;
        variance += diff * diff;
    }
    variance /= window_size;

    // Determine if this position should be a chunk boundary
    bool is_boundary = false;
    if (idx > 0) {
        float prev_value = data[idx - 1];
        float current_value = data[idx];
        float value_diff = abs(current_value - prev_value);
        float range = window_max - window_min;

        is_boundary = (value_diff > threshold * range) && (variance > threshold * window_mean);
    }

    chunk_boundaries[idx] = is_boundary ? 1 : 0;
}

template <typename T>
class GPUChunking {
private:
    int window_size;
    float threshold;
    cudaStream_t stream;

    // Helper to allocate GPU memory
    template <typename U>
    U* allocate_device_memory(size_t size) {
        U* d_ptr;
        CUDA_CHECK(cudaMalloc(&d_ptr, size * sizeof(U)));
        return d_ptr;
    }

    // Helper to copy data to GPU
    template <typename U>
    void copy_to_device(U* d_ptr, const U* h_ptr, size_t size) {
        CUDA_CHECK(cudaMemcpyAsync(d_ptr, h_ptr, size * sizeof(U), cudaMemcpyHostToDevice, stream));
    }

    // Helper to copy data from GPU
    template <typename U>
    void copy_from_device(U* h_ptr, const U* d_ptr, size_t size) {
        CUDA_CHECK(cudaMemcpyAsync(h_ptr, d_ptr, size * sizeof(U), cudaMemcpyDeviceToHost, stream));
    }

public:
    GPUChunking(int window_sz = 32, float thresh = 0.1f)
        : window_size(window_sz), threshold(thresh) {
        CUDA_CHECK(cudaStreamCreate(&stream));
    }

    ~GPUChunking() {
        cudaStreamDestroy(stream);
    }

    std::vector<std::vector<T>> chunk(const std::vector<T>& data) {
        if (data.empty())
            return {};

        // Allocate device memory
        int* d_data = allocate_device_memory<int>(data.size());
        int* d_boundaries = allocate_device_memory<int>(data.size());

        // Copy input data to GPU
        copy_to_device(d_data, data.data(), data.size());

        // Configure kernel launch parameters
        const int BLOCK_SIZE = 256;
        int num_blocks = (data.size() + BLOCK_SIZE - 1) / BLOCK_SIZE;

        // Launch kernel
        chunk_kernel<<<num_blocks, BLOCK_SIZE, 0, stream>>>(d_data, d_boundaries, data.size(),
                                                            window_size, threshold);

        // Check for kernel errors
        CUDA_CHECK(cudaGetLastError());

        // Copy boundaries back to host
        std::vector<int> boundaries(data.size());
        copy_from_device(boundaries.data(), d_boundaries, data.size());

        // Synchronize stream
        CUDA_CHECK(cudaStreamSynchronize(stream));

        // Free device memory
        CUDA_CHECK(cudaFree(d_data));
        CUDA_CHECK(cudaFree(d_boundaries));

        // Create chunks based on boundaries
        std::vector<std::vector<T>> chunks;
        std::vector<T> current_chunk;

        for (size_t i = 0; i < data.size(); ++i) {
            current_chunk.push_back(data[i]);
            if (boundaries[i] || i == data.size() - 1) {
                if (!current_chunk.empty()) {
                    chunks.push_back(std::move(current_chunk));
                    current_chunk = std::vector<T>();
                }
            }
        }

        return chunks;
    }

    // Getters and setters
    void set_window_size(int size) {
        if (size <= 0) {
            throw std::invalid_argument("Window size must be positive");
        }
        window_size = size;
    }

    void set_threshold(float thresh) {
        if (thresh <= 0.0f || thresh >= 1.0f) {
            throw std::invalid_argument("Threshold must be between 0 and 1");
        }
        threshold = thresh;
    }

    int get_window_size() const {
        return window_size;
    }
    float get_threshold() const {
        return threshold;
    }

    // Check if CUDA is available
    static bool is_gpu_available() {
        int device_count;
        cudaError_t error = cudaGetDeviceCount(&device_count);
        return (error == cudaSuccess) && (device_count > 0);
    }

    // Get GPU properties
    static std::string get_gpu_info() {
        if (!is_gpu_available()) {
            return "No CUDA-capable GPU found";
        }

        cudaDeviceProp prop;
        CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));

        return std::string("GPU Device: ") + prop.name +
               "\nCompute capability: " + std::to_string(prop.major) + "." +
               std::to_string(prop.minor);
    }
};

} // namespace gpu_chunking