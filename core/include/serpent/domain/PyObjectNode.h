#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <msgpack.hpp>

namespace serpent {
namespace domain {

struct PyObjectNode {
    uint64_t addr;          // Absolute address (key)
    uint32_t type_id;       // Integerized by IPythonABI (which manages a TypePool internally)
    uint32_t size;          // Byte size
    uint32_t flags;         // GC flags, etc.
    std::vector<uint64_t> refs;   // Referenced addresses (unordered)
    // --- Hash for change detection (to speed up diff even with large values)
    uint32_t content_hash;  // 16/32-bit rolling hash of the first N bytes

    // MessagePack serialization
    MSGPACK_DEFINE_ARRAY(addr, type_id, size, flags, refs, content_hash);
};

} // namespace domain
} // namespace serpent
