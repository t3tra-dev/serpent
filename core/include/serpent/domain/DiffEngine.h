#pragma once

#include <vector>
#include <utility>
#include <memory>
#include "serpent/domain/Snapshot.h"
#include "serpent/domain/PyObjectNode.h"

namespace serpent {
namespace domain {

struct DiffSet {
    std::vector<uint64_t> added;
    std::vector<uint64_t> removed;
    std::vector<uint64_t> type_changed;    // Nodes whose type_id changed
    std::vector<uint64_t> content_changed; // Nodes whose content_hash changed
    std::vector<uint64_t> references_structurally_changed; // Nodes where the set of referenced addresses changed
};

class DiffEngine {
public:
    DiffEngine() = default;
    ~DiffEngine() = default;

    // Disable copy and move semantics
    DiffEngine(const DiffEngine&) = delete;
    DiffEngine& operator=(const DiffEngine&) = delete;
    DiffEngine(DiffEngine&&) = delete;
    DiffEngine& operator=(DiffEngine&&) = delete;

    static DiffSet compare(const Snapshot& old_snapshot, const Snapshot& new_snapshot);
    // Overload for shared_ptr if SnapshotManager provides them
    static DiffSet compare(std::shared_ptr<const Snapshot> old_snapshot, std::shared_ptr<const Snapshot> new_snapshot);

private:
    // Helper methods for comparison, if needed
};

} // namespace domain
} // namespace serpent
