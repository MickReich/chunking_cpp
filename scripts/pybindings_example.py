import sys
import os

# Add the directory containing the compiled module to Python path
sys.path.append(os.path.join(os.path.dirname(os.path.dirname(__file__)), 'build/python'))

from chunking_cpp import chunking_cpp as cc
import numpy as np

def main():
    # Create sample data
    data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]

    # Basic chunking
    chunk = cc.Chunk(2)
    chunk.add(data)
    chunks = chunk.get_chunks()
    print("Basic chunks:", chunks)

    # Neural chunking
    neural_chunker = cc.NeuralChunking(3, 0.5)
    neural_chunks = neural_chunker.chunk(data)
    print("Neural chunks:", neural_chunks)

    # Sophisticated chunking
    wavelet_chunker = cc.WaveletChunking(8, 0.5)
    wavelet_chunks = wavelet_chunker.chunk()                                               ata)
    print("Wavelet chunks:", wavelet_chunks)
    
    # Metrics
    analyzer = cc.ChunkQualityAnalyzer()
    quality = analyzer.compute_quality_score(chunks)
    print("Chunk quality:", quality)

    # Visualization
    visualizer = cc.ChunkVisualizer(data, "./viz")
    visualizer.plot_chunk_sizes()

    # DTW chunking
    dtw_chunker = cc.DTWChunking(10, 1.0)
    dtw_chunks = dtw_chunker.chunk(data)
    print("DTW chunks:", dtw_chunks)
    
    # Benchmarking  
    benchmark = cc.ChunkBenchmark(data, "./benchmark_results")
    benchmark.run_benchmark()
    benchmark.save_results()

if __name__ == "__main__":
    main()