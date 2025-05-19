#include "serpent/domain/ObjectGraph.h"
#include "serpent/core/IProcessReader.h" 
#include "serpent/core/IPythonABI.h"   
#include "serpent/core/MemRegion.h"

#include <iostream>     
#include <vector>       
#include <unordered_set> 
#include <optional>     
#include <deque>

namespace serpent {
namespace domain {

// Definition of PyObject_HEAD structure elements needed by IPythonABI
// This is conceptual; actual parsing happens within IPythonABI methods.
// struct PyObject_HEAD_Minimal {
//     void* _ob_next; // Not always present or at this position
//     void* _ob_prev; // Not always present or at this position
//     size_t ob_refcnt; // Example, actual layout varies
//     uint64_t ob_type; // Address of the type object
// };

bool ObjectGraph::build(serpent::core::IProcessReader& reader, serpent::core::IPythonABI& abi, const std::vector<serpent::core::MemRegion>& memory_regions) {
    _nodes.clear();
    std::unordered_set<uint64_t> processed_addresses;

    const size_t pyobject_head_size = abi.get_pyobject_head_size();
    if (pyobject_head_size == 0) {
        std::cerr << "ObjectGraph::build: Error: PyObject_HEAD size from ABI is 0." << std::endl;
        return false;
    }
    std::vector<char> head_buffer(pyobject_head_size);
    const size_t alignment = sizeof(void*); // Typically pointer-size alignment

    std::vector<serpent::core::MemRegion> regions_to_scan = memory_regions;
    if (regions_to_scan.empty()) {
        regions_to_scan = reader.regions();
    }

    std::cout << "ObjectGraph::build: Starting memory scan. Regions: " << regions_to_scan.size()
              << ", PyObject_HEAD size: " << pyobject_head_size
              << ", Alignment: " << alignment << std::endl;

    for (const auto& region : regions_to_scan) {
        // Potentially filter regions based on permissions (e.g., must be readable)
        // std::cout << "Scanning region: " << std::hex << region.start << " - " << region.end << std::dec
        //           << " Perms: " << region.permissions << std::endl;

        for (uint64_t p = region.start; (p + pyobject_head_size) <= region.end; p += alignment) {
            if (p % alignment != 0) { // Ensure starting address 'p' is aligned
                // This check might be redundant if region.start is aligned and p is incremented by alignment.
                // However, if region.start is not necessarily aligned, we might want to align p first.
                // For simplicity, assuming p starts aligned or alignment handles it.
            }

            if (processed_addresses.count(p)) {
                continue; // Already processed this address (e.g., by a future BFS pass)
            }

            if (!reader.read(p, head_buffer.data(), pyobject_head_size)) { // Corrected to use read()
                // Failed to read memory at this specific location, common for inaccessible pages
                // std::cerr << "ObjectGraph::build: Failed to read PyObject_HEAD at 0x" << std::hex << p << std::dec << std::endl;
                continue;
            }

            uint64_t ob_type_addr = abi.get_ob_type_from_head_buffer(head_buffer.data(), head_buffer.size());
            if (ob_type_addr == 0) { // Invalid type address or ABI parsing error
                continue;
            }

            // This involves checking Py_TPFLAGS_TYPE_OBJECT and potentially libpython .data/.bss range (handled by ABI)
            if (abi.is_type_object(ob_type_addr, reader)) {
                PyObjectNode node;
                node.addr = p;
                node.type_id = abi.get_type_id_from_type_addr(ob_type_addr, reader);

                // Handle potential error from get_type_id_from_type_addr
                if (node.type_id == static_cast<uint32_t>(-1)) {
                    // std::cerr << "ObjectGraph::build: Failed to get valid type_id for object at 0x" << std::hex << p << " type_addr 0x" << ob_type_addr << std::dec << std::endl;
                    continue; // Skip this node if type_id is invalid
                }

                node.size = abi.get_object_size(p, ob_type_addr, reader);

                // Basic validation for size (e.g., non-zero, within reasonable limits)
                // A more sophisticated check might involve type-specific size heuristics in the ABI.
                if (node.size == 0 || node.size > 1024 * 1024 * 100) { // Heuristic: size 0 or >100MB is suspicious
                    // std::cerr << "ObjectGraph::build: Suspicious size " << node.size << " for object at 0x" << std::hex << p << std::dec << std::endl;
                    continue; // Likely a false positive or corrupted data
                }

                // Get GC flags or other relevant flags from the object/head
                node.flags = abi.get_object_flags(p, head_buffer.data(), head_buffer.size(), reader);

                node.refs = abi.get_references(p, ob_type_addr, reader);

                // Content Hash: using first N bytes. N is defined by the ABI or a constant.
                const size_t n_bytes_for_hash = 64; // Use first 64 bytes for hash
                node.content_hash = abi.calculate_content_hash(p, node.size, reader, n_bytes_for_hash);

                _nodes[p] = node;
                processed_addresses.insert(p);
                // std::cout << "ObjectGraph::build: Found PyObject at 0x" << std::hex << p << std::dec << " TypeID: " << node.type_id << " Size: " << node.size << std::endl;
            }
        }
    }

    std::cout << "ObjectGraph::build (heuristic scan phase) finished. Nodes found: " << _nodes.size() << std::endl;

    // --- BFS Expansion Phase ---
    std::cout << "ObjectGraph::build: Starting BFS expansion phase." << std::endl;
    std::vector<uint64_t> bfs_roots = abi.get_bfs_roots(reader);
    std::deque<uint64_t> queue;

    for (uint64_t root_addr : bfs_roots) {
        if (processed_addresses.find(root_addr) == processed_addresses.end()) {
            queue.push_back(root_addr);
            processed_addresses.insert(root_addr); // Mark as to-be-processed by BFS
        }
    }

    size_t bfs_nodes_found = 0;
    while (!queue.empty()) {
        uint64_t current_addr = queue.front();
        queue.pop_front();

        if (_nodes.count(current_addr)) {
            const auto& existing_node = _nodes.at(current_addr);
            for (uint64_t ref_addr : existing_node.refs) {
                if (processed_addresses.find(ref_addr) == processed_addresses.end()) {
                    queue.push_back(ref_addr);
                    processed_addresses.insert(ref_addr);
                }
            }
            continue;
        }
        
        // Process the object if not found by heuristic scan
        std::vector<char> current_head_buffer(pyobject_head_size);
        if (!reader.read(current_addr, current_head_buffer.data(), pyobject_head_size)) {
            // std::cerr << "ObjectGraph::build (BFS): Failed to read PyObject_HEAD at 0x" << std::hex << current_addr << std::dec << std::endl;
            continue;
        }

        uint64_t ob_type_addr = abi.get_ob_type_from_head_buffer(current_head_buffer.data(), current_head_buffer.size());
        if (ob_type_addr == 0) {
            continue;
        }

        if (abi.is_type_object(ob_type_addr, reader)) {
            PyObjectNode node;
            node.addr = current_addr;
            node.type_id = abi.get_type_id_from_type_addr(ob_type_addr, reader);

            if (node.type_id == static_cast<uint32_t>(-1)) {
                // std::cerr << "ObjectGraph::build (BFS): Failed to get valid type_id for object at 0x" << std::hex << current_addr << std::dec << std::endl;
                continue;
            }

            node.size = abi.get_object_size(current_addr, ob_type_addr, reader);
            if (node.size == 0 || node.size > 1024 * 1024 * 100) { // Same heuristic
                // std::cerr << "ObjectGraph::build (BFS): Suspicious size " << node.size << " for object at 0x" << std::hex << current_addr << std::dec << std::endl;
                continue;
            }

            node.flags = abi.get_object_flags(current_addr, current_head_buffer.data(), current_head_buffer.size(), reader);
            node.refs = abi.get_references(current_addr, ob_type_addr, reader);
            const size_t n_bytes_for_hash = 64; 
            node.content_hash = abi.calculate_content_hash(current_addr, node.size, reader, n_bytes_for_hash);

            _nodes[current_addr] = node;
            bfs_nodes_found++;
            // No need to insert to processed_addresses here as it was done when adding to queue

            for (uint64_t ref_addr : node.refs) {
                if (processed_addresses.find(ref_addr) == processed_addresses.end()) {
                    queue.push_back(ref_addr);
                    processed_addresses.insert(ref_addr);
                }
            }
        }
    }
    std::cout << "ObjectGraph::build (BFS expansion phase) finished. Additional nodes found by BFS: " << bfs_nodes_found << std::endl;

    // TODO: Implement BFS Expansion as per PHASE2.md
    // 1. Get root candidates from ABI: std::vector<uint64_t> roots = abi.get_bfs_roots(reader);
    // 2. For each root, perform BFS traversal:
    //    bfs_traversal(root_addr, reader, abi, processed_addresses, _nodes);
    //    The BFS traversal would also use processed_addresses to avoid re-processing and _nodes to add new finds.

    // std::cout << "ObjectGraph::build (heuristic scan phase) finished. Nodes found: " << _nodes.size() << std::endl;
    std::cout << "ObjectGraph::build (total after BFS) finished. Total nodes found: " << _nodes.size() << std::endl;
    if (_nodes.empty() && !memory_regions.empty() && bfs_roots.empty()) { // Adjusted condition
        std::cout << "ObjectGraph::build: Warning: No objects found after scanning memory regions and no BFS roots." << std::endl;
    }
    return true; // Consider returning false if critical errors occur or no objects are found when expected.
}

} // namespace domain
} // namespace serpent
