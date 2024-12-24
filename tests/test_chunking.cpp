TEST_CASE("Basic chunking operations") {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    chunk_processing::Chunk<double> chunker(2);
    chunker.add(data);
    auto chunks = chunker.get_chunks();
    REQUIRE(chunks.size() == 3);
}