#include <memory>
#include <string>
#include <dlfcn.h>
#include "serpent/core/IPythonABI.h"
#include "serpent/core/IProcessReader.h"

namespace serpent {
namespace core {

using create_abi_t = serpent::core::IPythonABI*(serpent::core::IProcessReader*);

std::unique_ptr<serpent::core::IPythonABI> create_abi_for_version(
    int major, int minor, serpent::core::IProcessReader* reader) {
    std::string suffix;
    #if defined(_WIN32)
        suffix = ".dll";
    #elif defined(__APPLE__)
        suffix = ".so";
    #else
        suffix = ".so";
    #endif
    
    std::string lib_name = "libserpent_abi_cp" + std::to_string(major) + std::to_string(minor) + suffix; // Assuming 'lib' prefix for shared libraries
    
    void* lib = dlopen(lib_name.c_str(), RTLD_NOW);
    if (!lib) {
        // Attempt without "lib" prefix for flexibility, or log error
        lib_name = "serpent_abi_cp" + std::to_string(major) + std::to_string(minor) + suffix;
        lib = dlopen(lib_name.c_str(), RTLD_NOW);
        if (!lib) {
             // Consider logging dlerror() here
            return nullptr; // Library not found
        }
    }
    
    auto fn = reinterpret_cast<create_abi_t*>(dlsym(lib, "create_abi"));
    if (!fn) {
        dlclose(lib);
        // Consider logging dlerror() here
        return nullptr; // Symbol not found
    }
    
    return std::unique_ptr<serpent::core::IPythonABI>(fn(reader));
}

} // namespace core
} // namespace serpent
