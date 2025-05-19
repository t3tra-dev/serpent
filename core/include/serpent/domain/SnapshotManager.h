#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include "serpent/domain/Snapshot.h"

namespace serpent {
namespace domain {

class SnapshotManager {
public:
    explicit SnapshotManager(size_t max_snapshots = 5);
    ~SnapshotManager() = default;

    // Disable copy and move semantics
    SnapshotManager(const SnapshotManager&) = delete;
    SnapshotManager& operator=(const SnapshotManager&) = delete;
    SnapshotManager(SnapshotManager&&) = delete;
    SnapshotManager& operator=(SnapshotManager&&) = delete;

    void addSnapshot(std::unique_ptr<Snapshot> snapshot);
    std::shared_ptr<const Snapshot> getSnapshot(size_t generation_index) const; // Get by index (0 is latest)
    std::shared_ptr<const Snapshot> getLatestSnapshot() const;
    size_t getSnapshotCount() const;
    size_t getMaxSnapshots() const { return _max_snapshots; }

private:
    std::deque<std::shared_ptr<Snapshot>> _snapshots; // Using shared_ptr for easier management with DiffEngine
    size_t _max_snapshots;
    mutable std::mutex _mutex; // For thread-safe access
};

} // namespace domain
} // namespace serpent
