#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <robin_hood.h>

#include "serpent/domain/PyObjectNode.h"
#include "serpent/core/MemRegion.h"

namespace serpent {
namespace core {
class IProcessReader;
class IPythonABI;
} 
} 

namespace serpent {
namespace domain {

class ObjectGraph {
public:
    using NodeMap = robin_hood::unordered_flat_map<uint64_t, PyObjectNode, std::hash<uint64_t>, std::equal_to<uint64_t>, 70>;

    ObjectGraph() = default;
    ObjectGraph(const ObjectGraph&) = delete;
    ObjectGraph& operator=(const ObjectGraph&) = delete;
    ObjectGraph(ObjectGraph&&) = default; // Changed from delete
    ObjectGraph& operator=(ObjectGraph&&) = default; // Changed from delete

    bool build(serpent::core::IProcessReader& reader, serpent::core::IPythonABI& abi, const std::vector<serpent::core::MemRegion>& memory_regions = {});
    
    const NodeMap& nodes() const noexcept { return _nodes; }
    NodeMap& nodes() noexcept { return _nodes; }

private:
    NodeMap _nodes;
};

} 
}
