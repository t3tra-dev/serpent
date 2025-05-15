#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace serpent {

class IPythonABI {
public:
    virtual ~IPythonABI() = default;
    virtual std::string type_name(uint64_t obj_addr) = 0;
    virtual size_t      size(uint64_t obj_addr) = 0;
    virtual std::vector<uint64_t> references(uint64_t obj_addr) = 0;
};

} // namespace serpent
