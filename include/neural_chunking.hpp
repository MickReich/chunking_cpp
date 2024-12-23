#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>

namespace neural_chunking {

/**
 * @brief Simple neural network layer for chunking decisions
 */
class Layer {
private:
    std::vector<std::vector<double>> weights;
    std::vector<double> biases;

    double sigmoid(double x) const {
        return 1.0 / (1.0 + std::exp(-x));
    }

public:
    Layer(size_t input_size, size_t output_size)
        : weights(output_size, std::vector<double>(input_size)), biases(output_size) {
        // Initialize weights with Xavier initialization
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> d(0.0, std::sqrt(2.0 / (input_size + output_size)));

        for (auto& neuron : weights) {
            for (auto& weight : neuron) {
                weight = d(gen);
            }
        }

        for (auto& bias : biases) {
            bias = d(gen);
        }
    }

    std::vector<double> forward(const std::vector<double>& input) const {
        if (input.size() != weights[0].size()) {
            throw std::invalid_argument("Input size mismatch");
        }

        std::vector<double> output(weights.size());
        for (size_t i = 0; i < weights.size(); ++i) {
            double sum = biases[i];
            for (size_t j = 0; j < input.size(); ++j) {
                sum += weights[i][j] * input[j];
            }
            output[i] = sigmoid(sum);
        }
        return output;
    }
};

/**
 * @brief Neural network based chunking strategy
 * @tparam T The type of elements to be chunked
 */
template <typename T>
class NeuralChunking {
private:
    std::vector<std::shared_ptr<Layer>> layers;
    size_t window_size;
    double threshold;

    /**
     * @brief Convert a window of data to neural network input
     * @param window Window of data to convert
     * @return Vector of normalized features
     */
    std::vector<double> extract_features(const std::vector<T>& window) const {
        if (window.empty())
            return {};

        std::vector<double> features;
        features.reserve(window.size());

        // Convert to double and normalize
        double min_val = static_cast<double>(*std::min_element(window.begin(), window.end()));
        double max_val = static_cast<double>(*std::max_element(window.begin(), window.end()));
        double range = max_val - min_val + 1e-10; // Avoid division by zero

        for (const auto& val : window) {
            features.push_back((static_cast<double>(val) - min_val) / range);
        }

        return features;
    }

    /**
     * @brief Predict if a boundary should be placed after the current window
     * @param window Current window of data
     * @return True if a boundary should be placed, false otherwise
     */
    bool predict_boundary(const std::vector<T>& window) const {
        if (window.size() != window_size) {
            throw std::invalid_argument("Invalid window size");
        }

        auto features = extract_features(window);
        auto current = features;

        // Forward propagation through all layers
        for (const auto& layer : layers) {
            current = layer->forward(current);
        }

        // Final output is probability of boundary
        return current.back() > threshold;
    }

public:
    /**
     * @brief Constructor for neural chunking
     * @param window_sz Size of the sliding window
     * @param boundary_threshold Threshold for boundary decision
     */
    NeuralChunking(size_t window_sz = 8, double boundary_threshold = 0.5)
        : window_size(window_sz), threshold(boundary_threshold) {
        // Create a simple neural network architecture
        layers.push_back(std::make_shared<Layer>(window_size, window_size * 2));
        layers.push_back(std::make_shared<Layer>(window_size * 2, window_size));
        layers.push_back(std::make_shared<Layer>(window_size, 1));
    }

    /**
     * @brief Chunk data using neural network predictions
     * @param data Input data to be chunked
     * @return Vector of chunks
     */
    std::vector<std::vector<T>> chunk(const std::vector<T>& data) const {
        if (data.empty()) {
            return {};
        }

        if (data.size() < window_size) {
            return {data};
        }

        std::vector<std::vector<T>> chunks;
        std::vector<T> current_chunk;

        for (size_t i = 0; i <= data.size() - window_size; ++i) {
            std::vector<T> window(data.begin() + i, data.begin() + i + window_size);
            current_chunk.push_back(data[i]);

            if (predict_boundary(window)) {
                if (!current_chunk.empty()) {
                    chunks.push_back(current_chunk);
                    current_chunk.clear();
                }
            }
        }

        // Add remaining elements
        for (size_t i = data.size() - window_size + 1; i < data.size(); ++i) {
            current_chunk.push_back(data[i]);
        }

        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        return chunks;
    }

    /**
     * @brief Update the boundary threshold
     * @param new_threshold New threshold value between 0 and 1
     */
    void set_threshold(double new_threshold) {
        if (new_threshold < 0.0 || new_threshold > 1.0) {
            throw std::invalid_argument("Threshold must be between 0 and 1");
        }
        threshold = new_threshold;
    }

    /**
     * @brief Get the current window size
     * @return Current window size
     */
    size_t get_window_size() const {
        return window_size;
    }
};

} // namespace neural_chunking