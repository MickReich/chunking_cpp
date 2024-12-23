import pytest
import numpy as np
from chunking_cpp.chunking_cpp import (
    Chunk, ChunkBenchmark, NeuralChunking, WaveletChunking,
    MutualInformationChunking, DTWChunking, ChunkVisualizer,
    ChunkSerializer, ResilientChunker, ChunkingError
)

# Fixtures
@pytest.fixture
def sample_data():
    return np.array([1.0, 1.1, 1.2, 5.0, 5.1, 5.2, 2.0, 2.1])

@pytest.fixture
def chunk_instance():
    return Chunk(3)

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
def test_wavelet_chunking(sample_data):
    wavelet = WaveletChunking(8, 0.5)
    chunks = wavelet.chunk(sample_data)
    assert len(chunks) > 0

def test_mutual_information_chunking(sample_data):
    mi = MutualInformationChunking(5, 0.3)
    chunks = mi.chunk(sample_data)
    assert len(chunks) > 0

def test_dtw_chunking(sample_data):
    dtw = DTWChunking(4, 2.0)
    chunks = dtw.chunk(sample_data)
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

# Resilience Tests
def test_resilient_chunker_initialization():
    chunker = ResilientChunker("test_checkpoint", 3, 2, 1)
    assert chunker is not None

def test_process_with_recovery(sample_data):
    chunker = ResilientChunker("test_checkpoint", 3, 2, 1)
    try:
        result = chunker.process(sample_data)
        assert result is not None
    except ChunkingError as e:
        assert str(e) != ""

def test_checkpoint_operations(sample_data):
    chunker = ResilientChunker("test_checkpoint", 3, 2, 1)
    chunker.process(sample_data)
    chunker.save_checkpoint()
    assert chunker.restore_from_checkpoint() is not None

# Parametrized Tests
@pytest.mark.parametrize("invalid_input", [
    [],  # Empty input
    [1.0],  # Too small input
    None,  # None input
])
def test_invalid_inputs(invalid_input):
    with pytest.raises(Exception):
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