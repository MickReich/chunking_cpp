/**
 * @file chunk_visualization.hpp
 * @brief Visualization utilities for chunk data
 * @author Jonathan Reich
 * @date 2024-12-07
 */

#pragma once
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace chunk_viz {

/**
 * @brief Class for visualizing chunk data in various formats
 * @tparam T The data type of the chunks (must be arithmetic)
 */
template <typename T>
class ChunkVisualizer {
private:
    std::vector<T>& chunks; ///< Reference to the chunks to visualize
    std::string output_dir; ///< Directory for output files

    /**
     * @brief Helper function to format chunk contents for visualization
     * @param chunk The chunk to format
     * @return Formatted string representation of the chunk
     */
    std::string format_chunk(const T& chunk) {
        if constexpr (std::is_same_v<T, std::vector<int>> ||
                      std::is_same_v<T, std::vector<double>>) {
            std::stringstream ss;
            ss << "[";
            for (size_t i = 0; i < chunk.size(); ++i) {
                ss << chunk[i];
                if (i < chunk.size() - 1)
                    ss << ",";
            }
            ss << "]";
            return ss.str();
        } else {
            return std::to_string(chunk);
        }
    }

public:
    /**
     * @brief Constructor
     * @param chunks_ Reference to the chunks to visualize
     * @param output_dir_ Directory for output files (default: "./viz")
     */
    explicit ChunkVisualizer(std::vector<T>& chunks_, const std::string& output_dir_ = "./viz")
        : chunks(chunks_), output_dir(output_dir_) {
        std::filesystem::create_directories(output_dir);
    }

    /**
     * @brief Generate plot of chunk sizes
     * @throws std::runtime_error if file operations fail
     */
    void plot_chunk_sizes() {
        // Create output directory if it doesn't exist
        std::filesystem::create_directories(output_dir);

        { // Use RAII scope blocks for file handles
            std::ofstream plot_data(output_dir + "/chunk_sizes.dat");
            if (!plot_data.is_open()) {
                throw std::runtime_error("Failed to create chunk_sizes.dat");
            }

            // Write chunk sizes
            for (size_t i = 0; i < chunks.size(); ++i) {
                if constexpr (std::is_same_v<T, std::vector<int>> ||
                              std::is_same_v<T, std::vector<double>>) {
                    plot_data << i << " " << chunks[i].size() << "\n";
                } else {
                    plot_data << i << " " << 1 << "\n"; // Single value counts as size 1
                }
            }
            plot_data.close(); // Explicitly close the file
        }

        { // Separate scope for gnuplot script
            std::ofstream gnuplot_script(output_dir + "/plot_chunks.gnu");
            if (!gnuplot_script.is_open()) {
                throw std::runtime_error("Failed to create plot_chunks.gnu");
            }

            gnuplot_script << "set terminal png\n"
                           << "set output '" << output_dir << "/chunk_sizes.png'\n"
                           << "set title 'Chunk Size Distribution'\n"
                           << "set xlabel 'Chunk Index'\n"
                           << "set ylabel 'Size'\n"
                           << "plot '" << output_dir
                           << "/chunk_sizes.dat' with lines title 'Chunk Sizes'\n";
            gnuplot_script.close(); // Explicitly close the file
        }
    }

    /**
     * @brief Visualize chunk boundaries in text format
     * @throws std::runtime_error if file operations fail
     */
    void visualize_boundaries() {
        std::ofstream viz_file(output_dir + "/boundaries.txt");

        size_t total_size = 0;
        for (const auto& chunk : chunks) {
            size_t chunk_size;
            if constexpr (std::is_same_v<T, std::vector<int>> ||
                          std::is_same_v<T, std::vector<double>>) {
                chunk_size = chunk.size();
            } else {
                chunk_size = 1; // Single value counts as size 1
            }
            // Draw boundary marker
            viz_file << "=== Chunk Boundary (size: " << chunk_size << ") ===\n";
            total_size += chunk_size;
        }

        viz_file << "\nTotal size: " << total_size << " elements\n";
    }

    /**
     * @brief Export chunk structure to GraphViz format
     * @param filename Optional custom filename for the output
     * @throws std::runtime_error if file operations fail
     */
    void export_to_graphviz(const std::string& filename = "") {
        std::string actual_filename = filename.empty() ? output_dir + "/chunks.dot" : filename;

        std::ofstream file(actual_filename);
        file << "digraph chunks {\n";

        for (size_t i = 0; i < chunks.size(); ++i) {
            file << "  chunk" << i << " [label=\"Chunk " << i
                 << "\\nValues: " << format_chunk(chunks[i]) << "\"];\n";
        }

        file << "}\n";
    }
};
} // namespace chunk_viz