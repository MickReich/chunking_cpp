import pytest
import numpy as np
from chunking_cpp.chunking_cpp import (
    Chunk, Chunk2D, Chunk3D, ChunkBenchmark, NeuralChunking, WaveletChunking,
    MutualInformationChunking, DTWChunking, ChunkVisualizer,
    ChunkSerializer, ChunkingError
)
import os
import tempfile
import shutil

# Fixtures
@pytest.fixture
def sample_data():
    return np.array([1.0, 1.1, 1.2, 5.0, 5.1, 5.2, 2.0, 2.1])

@pytest.fixture
def chunk_instance():
    return Chunk(3)

@pytest.fixture
def temp_viz_dir():
    """Create a temporary directory for visualization tests"""
    temp_dir = tempfile.mkdtemp()
    yield temp_dir
    shutil.rmtree(temp_dir)

@pytest.fixture
def chunker_setup():
    """Common setup for chunking tests"""
    data = [1.0, 2.0, 3.0, 4.0, 5.0]
    return data

# Basic Tests
def test_chunk_initialization():
    chunk = Chunk(3)
    assert chunk is not None

def test_add_single_element(chunk_instance):
    chunk_instance.add(1.0)
    chunks = chunk_instance.chunk_by_size(1)
    assert len(chunks) == 1
    assert chunks[0][0] == 1.0

def test_add_multiple_elements(chunk_instance, sample_data):
    chunk_instance.add(sample_data)
    chunks = chunk_instance.chunk_by_size(2)
    assert len(chunks) == 4

def test_chunk_by_threshold(chunk_instance, sample_data):
    chunk_instance.add(sample_data)
    chunks = chunk_instance.chunk_by_threshold(3.0)
    assert len(chunks) > 0

# Neural Chunking Tests
def test_neural_chunking_initialization():
    neural = NeuralChunking(8, 0.5)
    assert neural is not None

def test_neural_chunking_process(sample_data):
    neural = NeuralChunking(4, 0.5)
    chunks = neural.chunk(sample_data)
    assert len(chunks) > 0

def test_set_threshold():
    neural = NeuralChunking(8, 0.5)
    neural.set_threshold(0.7)
    assert neural.get_window_size() == 8

# Sophisticated Chunking Tests
def test_wavelet_chunking(chunker_setup):
    chunks = create_and_test_chunker(WaveletChunking(8, 0.5), chunker_setup)
    assert len(chunks) > 0

def test_mutual_information_chunking(sample_data):
    mi = MutualInformationChunking(5, 0.3)
    chunks = mi.chunk(sample_data)
    assert len(chunks) > 0

def test_dtw_chunking(chunker_setup):
    chunks = create_and_test_chunker(DTWChunking(4, 2.0), chunker_setup)
    assert len(chunks) > 0

# Serialization Tests
def test_serializer_initialization():
    serializer = ChunkSerializer()
    assert serializer is not None

def test_json_serialization(sample_data):
    serializer = ChunkSerializer()
    chunk_instance = Chunk(3)
    chunk_instance.add(sample_data)
    chunks = chunk_instance.chunk_by_size(2)
    try:
        json_data = serializer.to_json(chunks)
        assert json_data is not None
    except RuntimeError:
        pytest.skip("JSON serialization not available")

# Parametrized Tests
@pytest.mark.parametrize("invalid_input", [
    [],  # Empty input
    [1.0],  # Too small input
    None,  # None input
])
def test_invalid_inputs(invalid_input):
    with pytest.raises(ValueError):
        chunk = Chunk(3)
        if invalid_input is not None:
            chunk.add(invalid_input)
        chunk.chunk_by_threshold(1.0)

def test_error_handling():
    with pytest.raises(ValueError):
        # Try to create invalid chunk size
        Chunk(0)

# Cleanup Fixture
@pytest.fixture(autouse=True)
def cleanup():
    yield
    # Cleanup after tests
    import shutil
    import os
    if os.path.exists("./benchmark_results"):
        shutil.rmtree("./benchmark_results")
    if os.path.exists("./test_checkpoint"):
        shutil.rmtree("./test_checkpoint")
    if os.path.exists("./test_viz"):
        shutil.rmtree("./test_viz")

# Add these new test functions

def test_visualizer_initialization(sample_data, temp_viz_dir):
    visualizer = ChunkVisualizer(sample_data, temp_viz_dir)
    assert os.path.exists(temp_viz_dir)

def test_plot_chunk_sizes(sample_data, temp_viz_dir):
    visualizer = ChunkVisualizer(sample_data, temp_viz_dir)
    try:
        visualizer.plot_chunk_sizes()
        # Check files exist and are not empty
        dat_file = os.path.join(temp_viz_dir, "chunk_sizes.dat")
        gnu_file = os.path.join(temp_viz_dir, "plot_chunks.gnu")
        assert os.path.exists(dat_file)
        assert os.path.exists(gnu_file)
        assert os.path.getsize(dat_file) > 0
        assert os.path.getsize(gnu_file) > 0
    except Exception as e:
        pytest.fail(f"Visualization failed: {str(e)}")

def test_visualize_boundaries(sample_data, temp_viz_dir):
    visualizer = ChunkVisualizer(sample_data, temp_viz_dir)
    visualizer.visualize_boundaries()
    assert os.path.exists(os.path.join(temp_viz_dir, "boundaries.dat"))

def test_export_to_graphviz(sample_data, temp_viz_dir):
    visualizer = ChunkVisualizer(sample_data, temp_viz_dir)
    visualizer.export_to_graphviz("chunk_graph.dot")
    assert os.path.exists(os.path.join(temp_viz_dir, "chunk_graph.dot"))

def test_neural_chunking_advanced():
    # Test with various window sizes and thresholds
    data = np.array([1.0, 1.1, 5.0, 5.1, 2.0, 2.1, 6.0, 6.1])
    window_sizes = [2, 4, 8]
    thresholds = [0.3, 0.5, 0.7]
    
    for window in window_sizes:
        for threshold in thresholds:
            neural = NeuralChunking(window, threshold)
            chunks = neural.chunk(data)
            assert len(chunks) > 0
            assert all(isinstance(chunk, np.ndarray) for chunk in chunks)

def test_wavelet_chunking_advanced(sample_data):
    wavelet = WaveletChunking(8, 0.5)
    chunks = wavelet.chunk(sample_data)
    assert len(chunks) > 0
    
    # Test different parameters
    wavelet.set_window_size(4)
    assert wavelet.get_window_size() == 4
    
    wavelet.set_threshold(0.3)
    assert wavelet.get_threshold() == 0.3

def test_mutual_information_edge_cases():
    # Test with various edge cases
    test_cases = [
        np.array([1.0, 1.0, 1.0, 5.0, 5.0, 5.0]),  # Clear separation
        np.array([1.0, 2.0, 3.0, 4.0, 5.0]),       # Gradual increase
        np.array([1.0, 1.1, 1.0, 1.1, 1.0])        # Small variations
    ]
    
    mi = MutualInformationChunking(3, 0.3)
    for case in test_cases:
        chunks = mi.chunk(case)
        assert len(chunks) > 0

def test_dtw_chunking_parameters():
    data = np.array([1.0, 1.1, 5.0, 5.1, 2.0, 2.1])
    window_sizes = [2, 4]
    thresholds = [1.0, 2.0]
    
    for window in window_sizes:
        for threshold in thresholds:
            dtw = DTWChunking(window, threshold)
            chunks = dtw.chunk(data)
            assert len(chunks) > 0

def test_chunk_serialization_formats(sample_data):
    serializer = ChunkSerializer()
    chunk = Chunk(3)
    chunk.add(sample_data)
    chunks = chunk.chunk_by_size(2)
    
    # Test JSON serialization
    try:
        json_data = serializer.to_json(chunks)
        assert json_data is not None
        assert isinstance(json_data, str)
    except RuntimeError:
        pytest.skip("JSON serialization not available")
    
    # Test other formats if available
    try:
        protobuf_data = serializer.to_protobuf(chunks)
        assert protobuf_data is not None
    except RuntimeError:
        pytest.skip("Protobuf serialization not available")

@pytest.mark.parametrize("window_size,threshold", [
    (2, 0.3),
    (4, 0.5),
    (8, 0.7)
])
def test_chunking_parameters(sample_data, window_size, threshold):
    # Test different chunking algorithms with various parameters
    algorithms = [
        lambda: NeuralChunking(window_size, threshold),
        lambda: WaveletChunking(window_size, threshold),
        lambda: MutualInformationChunking(window_size, threshold),
        lambda: DTWChunking(window_size, threshold)
    ]
    
    for create_algorithm in algorithms:
        chunker = create_algorithm()
        chunks = chunker.chunk(sample_data)
        assert len(chunks) > 0
        assert all(len(chunk) > 0 for chunk in chunks)

def test_empty_and_edge_cases():
    # Test handling of empty input
    with pytest.raises(ValueError):
        chunk = Chunk(3)
        chunk.chunk_by_size(1)
    
    # Test single element
    chunk = Chunk(3)
    chunk.add(1.0)
    result = chunk.chunk_by_size(1)
    assert len(result) == 1
    
    # Test threshold edge cases
    chunk = Chunk(3)
    chunk.add([1.0, 1.0, 1.0])  # Identical values
    result = chunk.chunk_by_threshold(0.1)
    assert len(result) > 0

def test_2d_array_chunking():
    data = np.array([[1.0, 2.0], [3.0, 4.0], [5.0, 6.0]], dtype=np.float64)
    chunker = Chunk2D(2)
    chunker.add(data)
    chunks = chunker.get_chunks()
    assert len(chunks) == 2
    assert len(chunks[0]) == 2  # First chunk has 2 rows
    assert len(chunks[0][0]) == 2  # Each row has 2 columns

def test_3d_array_chunking():
    data = np.array([[[1.0, 2.0], [3.0, 4.0]],
                    [[5.0, 6.0], [7.0, 8.0]]], dtype=np.float64)
    chunker = Chunk3D(1)
    chunker.add(data)
    chunks = chunker.get_chunks()
    assert len(chunks) == 2
    assert len(chunks[0]) == 1  # Each chunk has 1 matrix
    assert len(chunks[0][0]) == 2  # Each matrix has 2 rows
    assert len(chunks[0][0][0]) == 2  # Each row has 2 columns

def create_and_test_chunker(chunker, data):
    """Helper function to test chunking algorithms"""
    return chunker.chunk(data)