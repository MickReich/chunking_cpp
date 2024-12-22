#pragma once
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>

namespace chunk_viz {
    template<typename T>
    class ChunkVisualizer {
    private:
        // Store reference to chunks or data structure
        std::vector<T>& chunks;
        std::string output_dir;

    public:
        // Constructor
        explicit ChunkVisualizer(std::vector<T>& chunks_, const std::string& output_dir_ = "./viz")
            : chunks(chunks_), output_dir(output_dir_) {}

        // ... existing declarations ...

        void plot_chunk_sizes() {
            // Create output directory if it doesn't exist
            std::filesystem::create_directories(output_dir);
            
            // Open output file for the plot data
            std::ofstream plot_data(output_dir + "/chunk_sizes.dat");
            
            // Write chunk sizes
            for (size_t i = 0; i < chunks.size(); ++i) {
                plot_data << i << " " << chunks[i].size() << "\n";
            }
            
            // Generate gnuplot script
            std::ofstream gnuplot_script(output_dir + "/plot_chunks.gnu");
            gnuplot_script << "set terminal png\n"
                          << "set output 'chunk_sizes.png'\n"
                          << "set title 'Chunk Size Distribution'\n"
                          << "set xlabel 'Chunk Index'\n"
                          << "set ylabel 'Size'\n"
                          << "plot 'chunk_sizes.dat' with lines title 'Chunk Sizes'\n";
        }

        void visualize_boundaries() {
            std::ofstream viz_file(output_dir + "/boundaries.txt");
            
            size_t total_size = 0;
            for (const auto& chunk : chunks) {
                // Draw boundary marker
                viz_file << "=== Chunk Boundary (size: " << chunk.size() << ") ===\n";
                total_size += chunk.size();
            }
            
            viz_file << "\nTotal size: " << total_size << " elements\n";
        }

        void export_to_graphviz() {
            std::ofstream dot_file(output_dir + "/chunks.dot");
            
            // Write DOT file header
            dot_file << "digraph ChunkVisualization {\n";
            dot_file << "  node [shape=record];\n";
            
            // Create nodes for each chunk
            for (size_t i = 0; i < chunks.size(); ++i) {
                dot_file << "  chunk" << i << " [label=\"Chunk " << i 
                        << "\\nSize: " << chunks[i].size() << "\"];\n";
                
                // Add edge to next chunk if not last
                if (i < chunks.size() - 1) {
                    dot_file << "  chunk" << i << " -> chunk" << (i + 1) << ";\n";
                }
            }
            
            dot_file << "}\n";
        }
    };
}