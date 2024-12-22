import sys
sys.path.append("../build/python")
import chunking_cpp as cc
import numpy as np
# Basic chunking
chunker = cc.Chunk(10)
data = [1.0, 2.0, 3.0, 4.0, 5.0]
chunker.add(data)
chunks = chunker.chunk_by_size(2)

# Neural chunking
neural_chunker = cc.NeuralChunking(8, 0.5)
neural_chunks = neural_chunker.chunk(data)

# Metrics
analyzer = cc.ChunkQualityAnalyzer()
quality = analyzer.compute_quality_score(chunks)

# Visualization
visualizer = cc.ChunkVisualizer(data, "./viz")
visualizer.plot_chunk_sizes()

# Resilient chunking
resilient = cc.ResilientChunker("./checkpoints", 1024*1024*1024, 1000, 5)
processed = resilient.process(data)

# Benchmarking
benchmark = cc.ChunkBenchmark(data, "./benchmark_results")
benchmark.run_benchmark()
benchmark.save_results()