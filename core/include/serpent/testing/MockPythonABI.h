#pragma once

#include "serpent/core/IPythonABI.h"
#include <gmock/gmock.h>
#include <string>
#include <vector>
#include <cstdint>
#include <map>

namespace serpent {
namespace testing {

class MockPythonABI : public serpent::core::IPythonABI {
public:
    // Mock methods from IPythonABI
    MOCK_METHOD(std::string, get_type_name, (uint64_t obj_addr, serpent::core::IProcessReader& reader), (override));
    MOCK_METHOD(size_t, get_object_size, (uint64_t obj_addr, uint64_t type_addr, serpent::core::IProcessReader& reader), (override));
    MOCK_METHOD(std::vector<uint64_t>, get_references, (uint64_t obj_addr, uint64_t type_addr, serpent::core::IProcessReader& reader), (override));
    MOCK_METHOD(uint32_t, get_object_flags, (uint64_t obj_addr, const void* head_buffer, size_t head_buffer_size, serpent::core::IProcessReader& reader), (override));

    MOCK_METHOD(size_t, get_pyobject_head_size, (), (const, override));
    MOCK_METHOD(uint64_t, get_ob_type_from_head_buffer, (const void* head_buffer, size_t head_buffer_size), (const, override));
    MOCK_METHOD(bool, is_type_object, (uint64_t type_addr, serpent::core::IProcessReader& reader), (override));
    // Renamed and new methods for TypePool
    MOCK_METHOD(uint32_t, get_type_id_by_name, (const std::string& type_name), (override));
    MOCK_METHOD(const std::string&, get_type_name_from_id, (uint32_t type_id), (const, override));
    MOCK_METHOD(void, clear_type_pool, (), (override));
    MOCK_METHOD(uint32_t, get_type_id_from_type_addr, (uint64_t type_addr, serpent::core::IProcessReader& reader), (override));

    MOCK_METHOD(uint32_t, calculate_content_hash, (uint64_t obj_addr, size_t obj_size, serpent::core::IProcessReader& reader, size_t n_bytes_for_hash), (override));

    MOCK_METHOD(std::vector<uint64_t>, get_bfs_roots, (serpent::core::IProcessReader& reader), (override));

    MOCK_METHOD(std::string, get_version_string, (), (const, override));
    MOCK_METHOD(int, get_major_version, (), (const, override));
    MOCK_METHOD(int, get_minor_version, (), (const, override));

    // Helper to manage type names for get_type_id and get_type_name_from_id
    uint32_t add_mock_type(uint64_t type_addr, const std::string& name) {
        _type_addr_to_id[type_addr] = _next_type_id;
        _id_to_type_name[_next_type_id] = name;
        _type_addr_to_name[type_addr] = name;
        return _next_type_id++;
    }

    uint32_t get_mock_type_id(uint64_t type_addr) {
        auto it = _type_addr_to_id.find(type_addr);
        if (it != _type_addr_to_id.end()) {
            return it->second;
        }
        // If not found, assign a new one for simplicity in some tests, or throw
        return add_mock_type(type_addr, "mock_type_" + std::to_string(type_addr));
    }

    std::string get_mock_type_name_from_addr(uint64_t type_addr) {
        auto it = _type_addr_to_name.find(type_addr);
        if (it != _type_addr_to_name.end()) {
            return it->second;
        }
        return "unknown_type_" + std::to_string(type_addr);
    }

private:
    uint32_t _next_type_id = 1;
    std::map<uint64_t, uint32_t> _type_addr_to_id;
    std::map<uint32_t, std::string> _id_to_type_name;
    std::map<uint64_t, std::string> _type_addr_to_name;
};

} // namespace testing
} // namespace serpent
