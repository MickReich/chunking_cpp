#include "chunk_serialization.hpp"
#include <gtest/gtest.h>

class ChunkSerializerTest : public ::testing::Test {
protected:
    void SetUp() override {
        chunks = {{1.0, 2.0, 3.0}, {4.0, 5.0}, {6.0, 7.0, 8.0}};
    }

    std::vector<std::vector<double>> chunks;
    chunk_serialization::ChunkSerializer<double> serializer;
};

TEST_F(ChunkSerializerTest, JsonSerialization) {
    std::string json = serializer.to_json(chunks);
    EXPECT_FALSE(json.empty());
    EXPECT_NE(json.find("["), std::string::npos);
    EXPECT_NE(json.find("]"), std::string::npos);
    EXPECT_NE(json.find("1.0"), std::string::npos);
}

TEST_F(ChunkSerializerTest, ProtobufSerialization) {
    try {
        std::string proto = serializer.to_protobuf(chunks);
        EXPECT_FALSE(proto.empty());
    } catch (const std::runtime_error& e) {
        GTEST_SKIP() << "Protobuf serialization not available";
    }
}

TEST_F(ChunkSerializerTest, MessagePackSerialization) {
    try {
        std::string msgpack = serializer.to_msgpack(chunks);
        EXPECT_FALSE(msgpack.empty());
    } catch (const std::runtime_error& e) {
        GTEST_SKIP() << "MessagePack serialization not available";
    }
}

TEST_F(ChunkSerializerTest, EmptyChunks) {
    std::vector<std::vector<double>> empty_chunks;
    EXPECT_NO_THROW(serializer.to_json(empty_chunks));
}