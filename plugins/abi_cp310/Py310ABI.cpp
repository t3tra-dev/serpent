// *超簡易実装*
#include "serpent/core/IPythonABI.h"
#include "serpent/core/IProcessReader.h"
#include <vector>
#include <string>
#include <unordered_map> // For TypePool
#include <stdexcept>
#include <iostream>

// Forward declaration of CPython internal structures
// These are simplified and might need adjustment based on actual CPython headers
// or more detailed information from the .c.md files.

namespace cpython_internal {
    // Simplified from internal/pycore_runtime.h
    struct _PyRuntimeState {
        // ... other fields ...
        // PyInterpreterState *interpreters_head; // Removed as per pystate.c
        // PyInterpreterState *current_interpreter; // Removed as per pystate.c
        // ... other fields ...
        uint64_t interpreters_head_addr; // Address of _PyRuntime.interpreters.head
                                         // This needs to be found, e.g. via symbol
    };

    // Simplified from internal/pycore_interp.h / pystate.h
    struct _is { // PyInterpreterState
        struct _is *next;
        struct _ts *threads;
        // ... many other fields ...
        // PyObject *modules; // This is PyDictObject*
        // PyObject *builtins; // This is PyDictObject*
        // PyObject *sysdict; // This is PyDictObject*
        uint64_t modules_addr;      // Offset or direct address of 'modules' dict
        uint64_t builtins_addr;     // Offset or direct address of 'builtins' dict
        uint64_t sysdict_addr;      // Offset or direct address of 'sysdict' dict

        // For finding these ^, we might need to know their offsets within PyInterpreterState
        // or have a way to get their addresses directly.
        // The .c.md files should provide clues.
        // For example, PyInterpreterState_GetDict (pystate.c) returns modules.
        // PyInterpreterState_GetBuiltins (pystate.c) returns builtins.
    };


    // Global variable holding the runtime state (e.g., _PyRuntime)
    // The address of this variable needs to be found, e.g., via symbol table.
    // For now, let's assume we can get its address.
    // static const uint64_t PYRUNTIME_ADDR = 0xADDRESS_OF_PYRUNTIME; // Placeholder
}


namespace serpent {

// These struct definitions are specific to this ABI implementation.
// It's fine for them to be in the serpent namespace if Py310ABI is also in serpent.
struct PyObject310 {
    uint64_t ob_refcnt;
    uint64_t ob_type;   // → PyTypeObject*
};

struct PyTypeObject310 {
    uint64_t ob_base[2];    // PyObject_HEAD
    uint64_t ob_size;       // This might be tp_basicsize in actual PyTypeObject
    uint64_t tp_name;       // const char*
    // ... other fields like tp_flags, tp_dictoffset etc. would be here
};

// Offsets within PyInterpreterState (these are illustrative and need to be accurate for CPython 3.10)
// These would be derived from CPython's source code (struct definitions).
// Example:
// const size_t OFFSET_INTERP_NEXT = offsetof(cpython_internal::PyInterpreterState, next);
// const size_t OFFSET_INTERP_MODULES = offsetof(cpython_internal::PyInterpreterState, modules);
// const size_t OFFSET_INTERP_BUILTINS = offsetof(cpython_internal::PyInterpreterState, builtins);
// const size_t OFFSET_INTERP_SYSDICT = offsetof(cpython_internal::PyInterpreterState, sysdict);

// For CPython 3.10, PyInterpreterState structure is complex.
// Let's assume we have a way to get the addresses of modules, builtins, and sysdict
// directly from a PyInterpreterState* (e.g., if they are stable members or accessible via functions
// whose logic we can replicate by reading memory).

// From Python/pystate.c, _PyRuntime.interpreters.head is the start.
// _PyRuntimeState _PyRuntime = {...};
// _PyRuntime.interpreters.head is a PyInterpreterState*

// From Python/pylifecycle.c, relevant functions for startup/shutdown might give clues
// on what's initialized and where.

// From Objects/object.c, _PyObject_GC_TRACK implementation might be relevant for GC roots.
// From Objects/typeobject.c, PyType_Ready and static types.

// For PyInterpreterState members in 3.10 (from internal/pycore_interp.h):
// PyObject *modules;
// PyObject *builtins;
// PyObject *sysdict;
// These are direct members. We need their offsets.
// Let's assume these offsets for now (replace with actual values from gdb or header analysis):
const size_t OFFSET_PYINTERPRETERSTATE_MODULES = 0x58; // Placeholder
const size_t OFFSET_PYINTERPRETERSTATE_BUILTINS = 0x68; // Placeholder
const size_t OFFSET_PYINTERPRETERSTATE_SYSDICT = 0x60; // Placeholder
const size_t OFFSET_PYINTERPRETERSTATE_NEXT = 0x8; // Placeholder for 'next' pointer in PyInterpreterState linked list.
const size_t OFFSET_PYRUNTIME_INTERPRETERS_HEAD = 0x20; // Placeholder for _PyRuntime.interpreters.head

// Address of the _PyRuntime global variable. This needs to be found via symbols.
// For testing, this could be a hardcoded value from a debugger session.
uint64_t _PyRuntime_addr = 0; // This should be initialized, perhaps via a constructor argument or a setter.

class Py310ABI final : public core::IPythonABI {
    core::IProcessReader* _reader; // Keep a pointer to the reader instance
    
    // TypePool implementation
    mutable std::unordered_map<std::string, uint32_t> _type_to_id_map;
    mutable std::vector<std::string> _id_to_type_name_list;
    mutable uint32_t _next_type_id = 0;
    uint64_t _runtime_addr; // To store the address of _PyRuntime

public:
    explicit Py310ABI(core::IProcessReader* r, uint64_t pyRuntimeAddr = 0) : _reader(r), _runtime_addr(pyRuntimeAddr) {
        // Initialize with common types or leave empty until first use
        _id_to_type_name_list.reserve(100); // Pre-allocate some space
        if (_runtime_addr == 0) {
            // In a real scenario, this would involve looking up the "_PyRuntime" symbol
            // For now, we'll print a warning. The caller should provide this.
            std::cerr << "Warning: _PyRuntime address not provided to Py310ABI. BFS roots will be empty." << std::endl;
        }
    }

    // --- Type Pool Management ---
    uint32_t get_type_id_by_name(const std::string& type_name) override {
        std::lock_guard<std::mutex> lock(_type_pool_mutex); // Assuming a mutex for thread safety if ABI can be shared
        auto it = _type_to_id_map.find(type_name);
        if (it != _type_to_id_map.end()) {
            return it->second;
        }
        uint32_t new_id = _next_type_id++;
        _type_to_id_map[type_name] = new_id;
        if (new_id >= _id_to_type_name_list.size()) {
            _id_to_type_name_list.resize(new_id + 1);
        }
        _id_to_type_name_list[new_id] = type_name;
        return new_id;
    }

    const std::string& get_type_name_from_id(uint32_t type_id) const override {
        std::lock_guard<std::mutex> lock(_type_pool_mutex);
        if (type_id >= _id_to_type_name_list.size() || _id_to_type_name_list[type_id].empty()) {
            // Or throw a more specific exception
            throw std::out_of_range("Type ID " + std::to_string(type_id) + " not found or not yet named.");
        }
        return _id_to_type_name_list[type_id];
    }
    
    void clear_type_pool() override {
        std::lock_guard<std::mutex> lock(_type_pool_mutex);
        _type_to_id_map.clear();
        _id_to_type_name_list.clear();
        _next_type_id = 0;
    }

    uint32_t get_type_id_from_type_addr(uint64_t type_addr, core::IProcessReader& reader) override {
        // This method now uses the existing get_type_name to find the string name,
        // then uses the TypePool to get/create an ID.
        // Note: get_type_name might be slow if called repeatedly for the same type_addr.
        // Consider caching type_addr -> type_id if performance becomes an issue.
        std::string name = get_type_name(type_addr, reader); // Assuming obj_addr for type_addr here for simplicity of get_type_name
        if (name.rfind("<err", 0) == 0 || name.rfind("<null",0) == 0) { // starts with <err or <null
             // Potentially return a special ID for errors or handle differently
            return static_cast<uint32_t>(-1); // Or some other error indicator
        }
        return get_type_id_by_name(name);
    }

    std::string get_type_name(uint64_t obj_addr, core::IProcessReader& reader) override {
        PyObject310 obj_head;
        if (!reader.read(obj_addr, &obj_head, sizeof(obj_head))) {
            return "<err_read_obj_head>";
        }
        if (obj_head.ob_type == 0) {
            return "<null_ob_type>";
        }

        PyTypeObject310 type_obj_struct;
        if (!reader.read(obj_head.ob_type, &type_obj_struct, sizeof(type_obj_struct))) {
            return "<err_read_type_struct>";
        }
        if (type_obj_struct.tp_name == 0) {
            return "<null_tp_name_ptr>";
        }

        char buf[256]{0}; // Initialize buffer to zeros
        // Read up to 255 chars to ensure null termination if string is longer
        if (!reader.read(type_obj_struct.tp_name, buf, sizeof(buf) - 1)) {
            return "<err_read_type_name_str>";
        }
        // buf is already null-terminated due to initialization or by read if string is shorter
        return buf;
    }

    size_t get_object_size(uint64_t /*obj_addr*/, uint64_t /*type_addr*/, core::IProcessReader& /*reader*/) override {
        // Placeholder: Actual implementation needed based on Python C API knowledge
        // e.g., read type_addr->tp_basicsize and potentially type_addr->tp_itemsize for varobjects
        return 32; // Arbitrary placeholder
    }

    std::vector<uint64_t> get_references(uint64_t /*obj_addr*/, uint64_t /*type_addr*/, core::IProcessReader& /*reader*/) override {
        // Placeholder: Actual implementation needed (e.g., scan object memory based on type)
        return {};
    }

    uint32_t get_object_flags(uint64_t /*obj_addr*/, const void* /*head_buffer*/, size_t /*head_buffer_size*/, core::IProcessReader& /*reader*/) override {
        // Placeholder: Read flags from PyTypeObject (e.g., tp_flags) or PyObject itself if applicable
        return 0;
    }

    size_t get_pyobject_head_size() const override {
        // This should be the size of PyObject_HEAD macro expansion for this Python version
        // For PyObject310 struct, it's the size of the struct itself if it accurately represents PyObject.
        // More accurately, it's offsetof(PyObject, ob_type) + sizeof(PyObject*.ob_type) if PyObject is standard.
        // Or, if PyObject_HEAD is just ob_refcnt and ob_type, then sizeof(PyObject310) is correct.
        return sizeof(PyObject310); 
    }

    uint64_t get_ob_type_from_head_buffer(const void* head_buffer, size_t head_buffer_size) const override {
        if (!head_buffer || head_buffer_size < sizeof(PyObject310)) { // Check against the known head structure
            return 0; // Error or insufficient data
        }
        // Assuming PyObject310 accurately represents the memory layout of PyObject_HEAD
        return reinterpret_cast<const PyObject310*>(head_buffer)->ob_type;
    }

    bool is_type_object(uint64_t type_addr, core::IProcessReader& reader) override {
        // Placeholder: Check if type_addr->ob_type == PyType_Type and/or check tp_flags & Py_TPFLAGS_TYPE_SUBCLASS
        // This requires knowing the address of PyType_Type or reading tp_flags.
        if (type_addr == 0) return false;
        // Read the PyTypeObject310 struct at type_addr
        PyTypeObject310 type_obj_struct;
        if (!reader.read(type_addr, &type_obj_struct, sizeof(type_obj_struct))) {
             return false; // Cannot read, assume not a type object
        }
        // A simple check could be if its ob_type points to what we assume is PyType_Type
        // This is a recursive and potentially problematic check without knowing PyType_Type's address.
        // A more robust check involves tp_flags. For now, returning a default.
        // Example (needs PyType_Type address and tp_flags offset):
        // uint64_t its_ob_type = type_obj_struct.ob_base[1]; // Assuming ob_base[1] is ob_type of the type object itself
        // if (its_ob_type == address_of_PyType_Type) return true;
        // uint32_t flags = /* read type_obj_struct.tp_flags */;
        // if (flags & Py_TPFLAGS_TYPE_SUBCLASS) return true; // Py_TPFLAGS_TYPE_SUBCLASS is usually (1UL << 9)
        return false; // Placeholder
    }

    uint32_t calculate_content_hash(uint64_t /*obj_addr*/, size_t /*obj_size*/, core::IProcessReader& /*reader*/, size_t /*n_bytes_for_hash*/) override {
        // Placeholder: Implement a rolling hash or similar over the first N bytes of the object's content.
        return 0;
    }

    std::vector<uint64_t> get_bfs_roots(core::IProcessReader& reader) override {
        std::vector<uint64_t> roots;
        if (_runtime_addr == 0) {
            std::cerr << "Error: _PyRuntime address is not set. Cannot get BFS roots." << std::endl;
            return roots; // Return empty if _PyRuntime address is not known
        }

        // 1. Get the head of the interpreters list from _PyRuntime.interpreters.head
        //    _PyRuntimeState *runtime = (PyRuntimeState*) _PyRuntime_addr;
        //    PyInterpreterState *interp_state = runtime->interpreters.head;
        uint64_t interpreters_head_ptr_addr = _runtime_addr + OFFSET_PYRUNTIME_INTERPRETERS_HEAD; // Address of the 'head' pointer itself
        uint64_t current_interp_state_addr = 0;
        if (!reader.read(interpreters_head_ptr_addr, &current_interp_state_addr, sizeof(current_interp_state_addr))) {
            std::cerr << "Failed to read interpreters_head_ptr_addr" << std::endl;
            return roots;
        }
        
        std::cout << "DEBUG: _PyRuntime address: 0x" << std::hex << _runtime_addr << std::endl;
        std::cout << "DEBUG: interpreters_head_ptr_addr: 0x" << std::hex << interpreters_head_ptr_addr << std::endl;
        std::cout << "DEBUG: Initial current_interp_state_addr: 0x" << std::hex << current_interp_state_addr << std::endl;


        // Iterate through the linked list of interpreters
        while (current_interp_state_addr != 0) {
            std::cout << "DEBUG: Processing interpreter state at: 0x" << std::hex << current_interp_state_addr << std::endl;
            // For each interpreter state, get modules, builtins, and sysdict
            // These are PyObject* (specifically PyDictObject*)

            // Address of PyInterpreterState->modules
            uint64_t modules_addr_ptr = current_interp_state_addr + OFFSET_PYINTERPRETERSTATE_MODULES;
            uint64_t modules_obj_addr = 0;
            if (reader.read(modules_addr_ptr, &modules_obj_addr, sizeof(modules_obj_addr)) && modules_obj_addr != 0) {
                roots.push_back(modules_obj_addr);
                std::cout << "DEBUG: Added modules_obj_addr: 0x" << std::hex << modules_obj_addr << std::endl;
            } else {
                std::cout << "DEBUG: Failed to read or null modules_obj_addr for interpreter 0x" << std::hex << current_interp_state_addr << std::endl;
            }

            // Address of PyInterpreterState->builtins
            uint64_t builtins_addr_ptr = current_interp_state_addr + OFFSET_PYINTERPRETERSTATE_BUILTINS;
            uint64_t builtins_obj_addr = 0;
            if (reader.read(builtins_addr_ptr, &builtins_obj_addr, sizeof(builtins_obj_addr)) && builtins_obj_addr != 0) {
                roots.push_back(builtins_obj_addr);
                 std::cout << "DEBUG: Added builtins_obj_addr: 0x" << std::hex << builtins_obj_addr << std::endl;
            } else {
                 std::cout << "DEBUG: Failed to read or null builtins_obj_addr for interpreter 0x" << std::hex << current_interp_state_addr << std::endl;
            }


            // Address of PyInterpreterState->sysdict
            uint64_t sysdict_addr_ptr = current_interp_state_addr + OFFSET_PYINTERPRETERSTATE_SYSDICT;
            uint64_t sysdict_obj_addr = 0;
            if (reader.read(sysdict_addr_ptr, &sysdict_obj_addr, sizeof(sysdict_obj_addr)) && sysdict_obj_addr != 0) {
                roots.push_back(sysdict_obj_addr);
                std::cout << "DEBUG: Added sysdict_obj_addr: 0x" << std::hex << sysdict_obj_addr << std::endl;
            } else {
                std::cout << "DEBUG: Failed to read or null sysdict_obj_addr for interpreter 0x" << std::hex << current_interp_state_addr << std::endl;
            }

            // Move to the next interpreter state
            // Address of PyInterpreterState->next
            uint64_t next_interp_state_ptr_addr = current_interp_state_addr + OFFSET_PYINTERPRETERSTATE_NEXT;
            if (!reader.read(next_interp_state_ptr_addr, &current_interp_state_addr, sizeof(current_interp_state_addr))) {
                std::cerr << "Failed to read next_interp_state_ptr_addr" << std::endl;
                break; // Stop if we can't read the next pointer
            }
            std::cout << "DEBUG: Next current_interp_state_addr: 0x" << std::hex << current_interp_state_addr << std::endl;
        }
        
        // Additionally, one might want to add well-known static type objects like PyType_Type, PyLong_Type etc.
        // if their addresses are known or can be discovered.
        // For example, if PyType_Type's address is known, add it.
        // This requires a mechanism to get these addresses (e.g. from symbols).

        // Also, consider GC roots if accessible (e.g., _PyGC_generation0).
        // This is more complex and might require parsing gc_heap structures.

        std::cout << "DEBUG: Total roots collected: " << roots.size() << std::endl;
        for(size_t i = 0; i < roots.size(); ++i) {
            std::cout << "DEBUG: Root [" << i << "]: 0x" << std::hex << roots[i] << std::endl;
        }
        // Reset cout to decimal for other outputs if necessary
        std::cout << std::dec; 

        return roots;
    }

    std::string get_version_string() const override { return "3.10.stub"; } // Corrected typo
    int get_major_version() const override { return 3; }
    int get_minor_version() const override { return 10; }

private:
    mutable std::mutex _type_pool_mutex; // Mutex for thread-safe access to TypePool members

};

// Factory function for the plugin
extern "C" serpent::core::IPythonABI* create_abi(serpent::core::IProcessReader* r) {
    // How to get _PyRuntime_addr here?
    // This is a critical piece of information.
    // For now, let's assume it's passed externally or known.
    // If serpent_reader_macos can find symbols, it could provide it.
    // As a placeholder, using the global _PyRuntime_addr which is 0 by default.
    // This means get_bfs_roots will likely fail or return empty unless _PyRuntime_addr is set.
    // A better approach would be for the tool using this ABI to discover and pass the address.
    // For testing, we might hardcode it if we find it with a debugger.
    // Let's modify the constructor to accept it.
    
    // uint64_t runtime_addr_from_somewhere = 0xYOUR_PYRUNTIME_ADDRESS; // Replace with actual logic or value
    // For now, defaulting to 0, which means BFS roots won't work unless set later or via a test.
    return new serpent::Py310ABI(r, _PyRuntime_addr);
}

} // namespace serpent
