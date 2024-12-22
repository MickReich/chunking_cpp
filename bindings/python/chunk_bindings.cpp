// Add Python bindings using pybind11
PYBIND11_MODULE(chunking_cpp, m) {
    py::class_<WaveletChunking<double>>(m, "WaveletChunking")
        .def(py::init<size_t, double>())
        .def("chunk", &WaveletChunking<double>::chunk);
    // ... other bindings
} 