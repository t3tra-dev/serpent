#include "serpent/domain/DiffEngine.h"
#include "serpent/domain/ObjectGraph.h"
#include <unordered_set>
#include <algorithm>
#include <iostream>

namespace serpent {
namespace domain {

DiffSet DiffEngine::compare(const Snapshot& old_snapshot, const Snapshot& new_snapshot) {
    std::cout << "DiffEngine: Comparing snapshots..." << std::endl;
    DiffSet diff_set;

    const auto& old_nodes = old_snapshot.getGraph().nodes();
    const auto& new_nodes = new_snapshot.getGraph().nodes();

    // 1. Iterate through new_nodes to find added or changed nodes
    for (const auto& pair_new : new_nodes) {
        const uint64_t addr = pair_new.first;
        const PyObjectNode& node_new = pair_new.second;

        auto it_old = old_nodes.find(addr);

        if (it_old == old_nodes.end()) {
            // Node is in new_snapshot but not in old_snapshot -> Added
            diff_set.added.push_back(addr);
        } else {
            // Node exists in both snapshots, check for changes
            const PyObjectNode& node_old = it_old->second;

            if (node_new.type_id != node_old.type_id) {
                diff_set.type_changed.push_back(addr);
            }

            else if (node_new.content_hash != node_old.content_hash) {
                diff_set.content_changed.push_back(addr);
            }

            std::unordered_set<uint64_t> old_refs_set(node_old.refs.begin(), node_old.refs.end());
            std::unordered_set<uint64_t> new_refs_set(node_new.refs.begin(), node_new.refs.end());

            if (old_refs_set != new_refs_set) {
                diff_set.references_structurally_changed.push_back(addr);
            }
        }
    }

    for (const auto& pair_old : old_nodes) {
        const uint64_t addr = pair_old.first;
        if (new_nodes.find(addr) == new_nodes.end()) {
            // Node is in old_snapshot but not in new_snapshot -> Removed
            diff_set.removed.push_back(addr);
        }
    }
    std::cout << "DiffEngine: Comparison complete. Added: " << diff_set.added.size()
              << ", Removed: " << diff_set.removed.size()
              << ", TypeChanged: " << diff_set.type_changed.size()
              << ", ContentChanged: " << diff_set.content_changed.size()
              << ", RefsChanged: " << diff_set.references_structurally_changed.size()
              << std::endl;

    return diff_set;
}

DiffSet DiffEngine::compare(std::shared_ptr<const Snapshot> old_snapshot, std::shared_ptr<const Snapshot> new_snapshot) {
    if (!old_snapshot || !new_snapshot) {
        std::cerr << "DiffEngine: Cannot compare null snapshots." << std::endl;
        return DiffSet(); // Return empty diff set
    }
    return compare(*old_snapshot, *new_snapshot);
}


} // namespace domain
} // namespace serpent
