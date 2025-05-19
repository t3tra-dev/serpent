#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace serpent { namespace core { class IProcessReader; } }

namespace serpent {
namespace core {

class IPythonABI {
public:
    virtual ~IPythonABI() = default;

    // --- Methods for PyObject introspection ---
    virtual std::string get_type_name(uint64_t obj_addr, IProcessReader& reader) = 0;
    virtual size_t get_object_size(uint64_t obj_addr, uint64_t type_addr, IProcessReader& reader) = 0;
    virtual std::vector<uint64_t> get_references(uint64_t obj_addr, uint64_t type_addr, IProcessReader& reader) = 0;
    virtual uint32_t get_object_flags(uint64_t obj_addr, const void* head_buffer, size_t head_buffer_size, IProcessReader& reader) = 0;

    // --- Methods for ObjectGraph::build ---
    virtual size_t get_pyobject_head_size() const = 0;
    virtual uint64_t get_ob_type_from_head_buffer(const void* head_buffer, size_t head_buffer_size) const = 0;
    virtual bool is_type_object(uint64_t type_addr, IProcessReader& reader) = 0;
    
    // --- Type Pool Management ---
    // Retrieves or creates a unique ID for a given type name.
    virtual uint32_t get_type_id_by_name(const std::string& type_name) = 0;
    // Retrieves the type name associated with a given ID. Throws if ID not found.
    virtual const std::string& get_type_name_from_id(uint32_t type_id) const = 0;
    // Clears all stored type information. Useful for testing or re-initialization.
    virtual void clear_type_pool() = 0;
    // Internal helper: gets type_id from type_addr, potentially populating TypePool
    virtual uint32_t get_type_id_from_type_addr(uint64_t type_addr, IProcessReader& reader) = 0;

    virtual uint32_t calculate_content_hash(uint64_t obj_addr, size_t obj_size, IProcessReader& reader, size_t n_bytes_for_hash) = 0;

    // --- Methods for BFS root finding ---
    virtual std::vector<uint64_t> get_bfs_roots(IProcessReader& reader) = 0;

    // --- Version information ---
    virtual std::string get_version_string() const = 0;
    virtual int get_major_version() const = 0;
    virtual int get_minor_version() const = 0;

    // --- Type Pool Management (conceptual, could be part of get_type_id or separate) ---
    // virtual void clear_type_pool() = 0;
    // virtual const std::string& get_type_name_from_id(uint32_t type_id) const = 0;
    // Renamed for clarity from: get_type_id(uint64_t type_addr, IProcessReader& reader)
    // This method will now internally use get_type_name and then get_type_id_by_name
};

} // namespace core
} // namespace serpent
