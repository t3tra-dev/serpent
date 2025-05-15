#include <memory>
#include <string>
#include <dlfcn.h>
#include "serpent/core/IPythonABI.h"
#include "serpent/core/IProcessReader.h"

namespace serpent {

using create_abi_t = IPythonABI*(IProcessReader*);

std::unique_ptr<IPythonABI> create_abi_for_version(int major, int minor, IProcessReader* reader) {
    // Construct the library name "serpent_abi_cpXY.so/.dylib/.dll"
    std::string suffix = ".so";
    #if defined(_WIN32)
    suffix = ".dll";
    #endif
    // Use .so even on macOS
    
    std::string lib_name = "serpent_abi_cp" + std::to_string(major) + std::to_string(minor) + suffix;
    
    void* lib = dlopen(lib_name.c_str(), RTLD_NOW);
    if (!lib) {
        return nullptr; // Library not found
    }
    
    auto fn = reinterpret_cast<create_abi_t*>(dlsym(lib, "create_abi"));
    if (!fn) {
        dlclose(lib);
        return nullptr; // Symbol not found
    }
    
    return std::unique_ptr<IPythonABI>(fn(reader));
}

} // namespace serpent
