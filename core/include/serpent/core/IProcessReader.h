#pragma once
#include <cstdint>
#include <vector>
#include "MemRegion.h"

namespace serpent::core {

class IProcessReader {
public:
    virtual ~IProcessReader() = default;
    virtual bool attach(int pid) = 0;
    virtual void detach() = 0;
    virtual bool read(uint64_t addr, void* buf, size_t len) = 0;
    virtual std::vector<MemRegion> regions() = 0;
};

} // namespace serpent::core
