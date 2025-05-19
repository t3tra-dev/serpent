#pragma once

#include <cstdint>
#include <string>

namespace serpent {
namespace core {

struct MemRegion {
    uint64_t start;
    uint64_t end;
    uint64_t size;
    uint32_t permissions;
    std::string name;
    // Add other relevant fields like offset, device, inode if necessary
};

} // namespace core
} // namespace serpent
