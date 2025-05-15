#include <dlfcn.h>
#include <memory>
#include <stdexcept>
#include <string>
#include "serpent/core/IProcessReader.h"
#include "serpent/core/ReaderFactory.h"

namespace serpent {

using create_reader_t = IProcessReader*();

std::unique_ptr<IProcessReader> create_reader_for_current_os() {
    // Determine the library name based on the current OS
    std::string lib_name;
    std::string suffix = ".so";
    
    #if defined(__linux__)
        lib_name = "serpent_reader_linux";
        suffix = ".so";
    #elif defined(__APPLE__)
        lib_name = "serpent_reader_macos";
        suffix = ".so";  // Use .so for macOS
    #elif defined(_WIN32)
        lib_name = "serpent_reader_windows";
        suffix = ".dll";
    #else
        throw std::runtime_error("Unsupported platform");
    #endif
    
    lib_name += suffix;
    
    void* lib = dlopen(lib_name.c_str(), RTLD_NOW);
    if (!lib) {
        throw std::runtime_error("Failed to load reader plugin: " + std::string(dlerror()));
    }
    
    auto fn = reinterpret_cast<create_reader_t*>(dlsym(lib, "create_reader"));
    if (!fn) {
        dlclose(lib);
        throw std::runtime_error("Failed to locate create_reader symbol");
    }
    
    return std::unique_ptr<IProcessReader>(fn());
}

} // namespace serpent
