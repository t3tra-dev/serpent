#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "serpent/domain/ObjectGraph.h"
#include "serpent/testing/MockProcessReader.h"
#include "serpent/testing/MockPythonABI.h"
#include "serpent/core/IProcessReader.h"

using namespace serpent::domain;
using namespace serpent::testing;
using namespace serpent::core;
using ::testing::AtLeast;
using ::testing::Return;
using ::testing::_;

class ObjectGraphBuildTest : public ::testing::Test {
protected:
    MockProcessReader mock_reader;
    MockPythonABI mock_abi;
    ObjectGraph graph;
    std::vector<MemRegion> regions;

    void SetUp() override {
        // Common setup for all tests in this fixture
        ON_CALL(mock_reader, read(_,_,_)).WillByDefault(Return(false)); // Default: fail reads
        ON_CALL(mock_reader, regions()).WillByDefault(Return(regions));
        ON_CALL(mock_abi, get_pyobject_head_size()).WillByDefault(Return(16)); // Example size
    }

    // Helper to create a MemRegion
    MemRegion make_region(uint64_t start, uint64_t size, uint32_t perms = 0x7) { // rwx = 0x7
        MemRegion region;
        region.start = start;
        region.end = start + size;
        region.size = size;
        region.permissions = perms;
        // region.name = ...; // if needed
        return region;
    }
};

TEST_F(ObjectGraphBuildTest, Build_EmptyMemoryRegions_ReturnsEmptyGraph) {
    regions.clear();
    EXPECT_CALL(mock_reader, regions()).WillOnce(Return(regions)); // Should be called by build if it iterates regions
    // No specific calls to read or ABI methods are expected if there are no regions to scan.

    bool success = graph.build(mock_reader, mock_abi, regions);
    ASSERT_TRUE(success); // Build itself should succeed
    EXPECT_TRUE(graph.nodes().empty());
}

TEST_F(ObjectGraphBuildTest, Build_SingleRegionNoObjects_ReturnsEmptyGraph) {
    // Set a smaller size to improve processing speed
    regions.push_back(make_region(0x1000, 0x100)); // Changed to a small 256-byte region
    // regions() should not be called (since memory_regions is explicitly passed)

    // Expect ABI calls for PyObject_HEAD size
    EXPECT_CALL(mock_abi, get_pyobject_head_size()).WillRepeatedly(Return(16)); // Example size

    // Add explicit EXPECT_CALL to limit the number of read calls
    // To align to 8-byte units, a 256-byte region allows at most 32 read calls
    EXPECT_CALL(mock_reader, read(_,_,_)).Times(testing::AtMost(32)).WillRepeatedly(Return(false));
    
    // ob_type_from_head_buffer should never be called (since read always fails)
    EXPECT_CALL(mock_abi, get_ob_type_from_head_buffer(_, _)).Times(0);

    bool success = graph.build(mock_reader, mock_abi, regions);
    ASSERT_TRUE(success);
    EXPECT_TRUE(graph.nodes().empty());
}

// TODO: Add more tests:
// - Test with a simple valid PyObject: mock reader to return valid head, mock ABI to validate type.
// - Test BFS traversal (once implemented).
// - Test content hash calculation.
// - Test various edge cases (misaligned data, read errors in the middle of objects etc.)

int main(int argc, char **argv) {
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

