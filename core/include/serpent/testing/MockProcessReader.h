#pragma once

#include "serpent/core/IProcessReader.h"
#include <gmock/gmock.h>
#include <map>
#include <vector>

namespace serpent {
namespace testing {

class MockProcessReader : public serpent::core::IProcessReader {
public:
    MOCK_METHOD(bool, attach, (pid_t pid), (override));
    MOCK_METHOD(void, detach, (), (override));
    MOCK_METHOD(bool, read, (uint64_t address, void* buf, size_t len), (override));
    MOCK_METHOD(std::vector<serpent::core::MemRegion>, regions, (), (override));

    // Helper methods for setting up mock memory
    void setMemoryRegion(uint64_t address, const std::vector<uint8_t>& data) {
        _memory_data[address] = data;
    }

    void addMemoryRegion(const serpent::core::MemRegion& region) {
        _regions.push_back(region);
    }

    // Custom action for read to simulate actual memory reading
    bool readMock(uint64_t address, void* buf, size_t len) {

        auto it = _memory_data.lower_bound(address);
        if (it != _memory_data.end()) {
            // Check if 'address' is within this stored block
            uint64_t block_start = it->first;
            const std::vector<uint8_t>& block_data = it->second;
            if (address >= block_start && (address + len) <= (block_start + block_data.size())) {
                memcpy(buf, block_data.data() + (address - block_start), len);
                return true;
            }
        }
        // A more sophisticated search: iterate through _memory_data to find if 'address' falls into any part of a stored block.
        for (const auto& pair : _memory_data) {
            uint64_t block_start = pair.first;
            const auto& block_data = pair.second;
            uint64_t block_end = block_start + block_data.size();
            if (address >= block_start && (address + len) <= block_end) {
                 memcpy(buf, block_data.data() + (address - block_start), len);
                 return true;
            }
        }
        return false; // Address not found or read out of bounds
    }

    std::vector<serpent::core::MemRegion> enumMemoryRegionsMock() {
        return _regions;
    }

private:
    std::map<uint64_t, std::vector<uint8_t>> _memory_data; // Stores chunks of memory data
    std::vector<serpent::core::MemRegion> _regions; // Stores defined memory regions
};

} // namespace testing
} // namespace serpent
