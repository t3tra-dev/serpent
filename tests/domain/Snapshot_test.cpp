#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fstream>
#include <cstdio>

#include "serpent/domain/Snapshot.h"
#include "serpent/domain/ObjectGraph.h"
#include "serpent/domain/PyObjectNode.h"

// Helper function to create a temporary filepath
std::string temp_filepath() {
    // Using tmpnam is generally unsafe, but for a test where the file is immediately used and deleted,
    // and the risk of collision is low, it might be acceptable.
    // A better approach would be to use mkstemp or a library function if available and portable.
    char name_buffer[L_tmpnam];
    if (std::tmpnam(name_buffer) == nullptr) {
        throw std::runtime_error("Failed to create temporary file name.");
    }
    return std::string(name_buffer);
}

class SnapshotTest : public ::testing::Test {
protected:
    std::string test_filepath;

    void SetUp() override {
        test_filepath = temp_filepath() + ".snapshot";
    }

    void TearDown() override {
        std::remove(test_filepath.c_str());
    }
};

TEST_F(SnapshotTest, EmptyGraphSerializationDeserialization) {
    serpent::domain::ObjectGraph empty_graph;
    uint64_t epoch_ms = 1234567890;
    uint32_t py_major = 3;
    uint32_t py_minor = 10;

    serpent::domain::Snapshot original_snapshot(epoch_ms, py_major, py_minor, std::move(empty_graph));

    ASSERT_TRUE(original_snapshot.serialize(test_filepath));

    std::unique_ptr<serpent::domain::Snapshot> deserialized_snapshot = serpent::domain::Snapshot::deserialize(test_filepath);
    ASSERT_NE(deserialized_snapshot, nullptr);

    // Check header
    EXPECT_EQ(deserialized_snapshot->getHeader().epoch_ms, epoch_ms);
    EXPECT_EQ(deserialized_snapshot->getHeader().py_major, py_major);
    EXPECT_EQ(deserialized_snapshot->getHeader().py_minor, py_minor);
    EXPECT_EQ(deserialized_snapshot->getHeader().node_count, 0);

    // Check graph
    EXPECT_TRUE(deserialized_snapshot->getGraph().nodes().empty());
}

TEST_F(SnapshotTest, SimpleGraphSerializationDeserialization) {
    serpent::domain::ObjectGraph graph;
    uint64_t epoch_ms = 9876543210;
    uint32_t py_major = 3;
    uint32_t py_minor = 8;

    // Node 1
    serpent::domain::PyObjectNode node1;
    node1.addr = 0x1000;
    node1.type_id = 1;
    node1.size = 32;
    node1.flags = 0;
    node1.refs = {0x2000, 0x3000};
    node1.content_hash = 123;
    graph.nodes()[node1.addr] = node1;

    // Node 2
    serpent::domain::PyObjectNode node2;
    node2.addr = 0x2000;
    node2.type_id = 2;
    node2.size = 64;
    node2.flags = 1;
    node2.refs = {};
    node2.content_hash = 456;
    graph.nodes()[node2.addr] = node2;
    
    // Node 3
    serpent::domain::PyObjectNode node3;
    node3.addr = 0x3000;
    node3.type_id = 1; // Same type as node1
    node3.size = 16;
    node3.flags = 0;
    node3.refs = {0x1000}; // Circular reference
    node3.content_hash = 789;
    graph.nodes()[node3.addr] = node3;

    serpent::domain::Snapshot original_snapshot(epoch_ms, py_major, py_minor, std::move(graph));
    ASSERT_TRUE(original_snapshot.serialize(test_filepath));

    std::unique_ptr<serpent::domain::Snapshot> deserialized_snapshot = serpent::domain::Snapshot::deserialize(test_filepath);
    ASSERT_NE(deserialized_snapshot, nullptr);

    // Check header
    EXPECT_EQ(deserialized_snapshot->getHeader().epoch_ms, epoch_ms);
    EXPECT_EQ(deserialized_snapshot->getHeader().py_major, py_major);
    EXPECT_EQ(deserialized_snapshot->getHeader().py_minor, py_minor);
    EXPECT_EQ(deserialized_snapshot->getHeader().node_count, 3);

    // Check graph
    const auto& deserialized_nodes = deserialized_snapshot->getGraph().nodes();
    ASSERT_EQ(deserialized_nodes.size(), 3);

    // Check node1
    auto it1 = deserialized_nodes.find(0x1000);
    ASSERT_NE(it1, deserialized_nodes.end());
    EXPECT_EQ(it1->second.type_id, node1.type_id);
    EXPECT_EQ(it1->second.size, node1.size);
    EXPECT_EQ(it1->second.flags, node1.flags);
    EXPECT_EQ(it1->second.refs, node1.refs);
    EXPECT_EQ(it1->second.content_hash, node1.content_hash);

    // Check node2
    auto it2 = deserialized_nodes.find(0x2000);
    ASSERT_NE(it2, deserialized_nodes.end());
    EXPECT_EQ(it2->second.type_id, node2.type_id);
    EXPECT_EQ(it2->second.size, node2.size);
    EXPECT_EQ(it2->second.flags, node2.flags);
    EXPECT_EQ(it2->second.refs, node2.refs);
    EXPECT_EQ(it2->second.content_hash, node2.content_hash);
    
    // Check node3
    auto it3 = deserialized_nodes.find(0x3000);
    ASSERT_NE(it3, deserialized_nodes.end());
    EXPECT_EQ(it3->second.type_id, node3.type_id);
    EXPECT_EQ(it3->second.size, node3.size);
    EXPECT_EQ(it3->second.flags, node3.flags);
    EXPECT_EQ(it3->second.refs, node3.refs);
    EXPECT_EQ(it3->second.content_hash, node3.content_hash);
}

TEST_F(SnapshotTest, NonExistentFileDeserialization) {
    std::string non_existent_filepath = "/tmp/this_file_should_not_exist_ever.snapshot";
    std::remove(non_existent_filepath.c_str()); // Ensure it doesn't exist

    std::unique_ptr<serpent::domain::Snapshot> deserialized_snapshot = serpent::domain::Snapshot::deserialize(non_existent_filepath);
    EXPECT_EQ(deserialized_snapshot, nullptr);
}

// TODO: Add InvalidFileFormatDeserialization test
// This might involve creating a file with bad header, corrupted zstd data, or invalid msgpack.

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
