#include "serpent/domain/SnapshotManager.h"
#include <algorithm>
#include <iostream>

namespace serpent {
namespace domain {

SnapshotManager::SnapshotManager(size_t max_snapshots)
    : _max_snapshots(max_snapshots > 0 ? max_snapshots : 1) { // Ensure at least 1 snapshot can be stored
    std::cout << "SnapshotManager initialized with max_snapshots: " << _max_snapshots << std::endl;
}

void SnapshotManager::addSnapshot(std::unique_ptr<Snapshot> snapshot) {
    if (!snapshot) {
        std::cerr << "SnapshotManager: Attempted to add a null snapshot." << std::endl;
        return;
    }
    std::lock_guard<std::mutex> lock(_mutex);
    if (_snapshots.size() >= _max_snapshots) {
        _snapshots.pop_back(); // Remove the oldest snapshot (from the end of the deque)
        std::cout << "SnapshotManager: Removed oldest snapshot to maintain max size." << std::endl;
    }
    _snapshots.push_front(std::move(snapshot)); // Add new snapshot to the front (most recent)
    std::cout << "SnapshotManager: Added new snapshot. Total snapshots: " << _snapshots.size() << std::endl;
}

std::shared_ptr<const Snapshot> SnapshotManager::getSnapshot(size_t generation_index) const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (generation_index < _snapshots.size()) {
        return _snapshots[generation_index];
    }
    std::cerr << "SnapshotManager: Snapshot index " << generation_index << " out of bounds." << std::endl;
    return nullptr; // Index out of bounds
}

std::shared_ptr<const Snapshot> SnapshotManager::getLatestSnapshot() const {
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_snapshots.empty()) {
        return _snapshots.front();
    }
    std::cout << "SnapshotManager: No snapshots available to get the latest." << std::endl;
    return nullptr; // No snapshots available
}

size_t SnapshotManager::getSnapshotCount() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _snapshots.size();
}

} // namespace domain
} // namespace serpent
