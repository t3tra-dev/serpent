#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "serpent/domain/ObjectGraph.h"

namespace serpent {
namespace domain {

struct SnapshotHeader {
    uint64_t epoch_ms;
    uint32_t py_major;
    uint32_t py_minor;
    uint32_t node_count;
    // add other metadata fields as needed
};

class Snapshot {
public:
    Snapshot(uint64_t epoch_ms, uint32_t py_major, uint32_t py_minor, ObjectGraph graph);
    ~Snapshot() = default;

    // Disable copy and move semantics for now, or implement properly if needed
    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;
    Snapshot(Snapshot&&) = delete;
    Snapshot& operator=(Snapshot&&) = delete;

    const SnapshotHeader& getHeader() const { return _header; }
    const ObjectGraph& getGraph() const { return _object_graph; }

    bool serialize(const std::string& filepath) const;
    static std::unique_ptr<Snapshot> deserialize(const std::string& filepath);

private:
    SnapshotHeader _header;
    ObjectGraph _object_graph;
};

} // namespace domain
} // namespace serpent
