#include <iostream>
#include <memory>
#include <stdexcept>
#include "serpent/core/IProcessReader.h"
#include "serpent/core/IPythonABI.h"
#include "serpent/core/ReaderFactory.h"
#include "serpent/core/AbiFactory.h"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <pid> <addr> [python_major python_minor]\n";
        std::cerr << "Example: " << argv[0] << " 1234 0x7ffabcdef000 3 10\n";
        return 1;
    }

    try {
        int pid = std::stoi(argv[1]);
        uint64_t addr = std::stoull(argv[2], nullptr, 16);
        
        // Python version (default is 3.10)
        int py_major = 3;
        int py_minor = 10;
        
        if (argc >= 5) {
            py_major = std::stoi(argv[3]);
            py_minor = std::stoi(argv[4]);
        }

        std::cout << "Attaching to process " << pid << "...\n";
        auto reader = serpent::create_reader_for_current_os();
        
        if (!reader) {
            std::cerr << "Error: Failed to create reader plugin for the OS\n";
            return 1;
        }
        
        if (!reader->attach(pid)) {
            std::cerr << "Error: Failed to attach to process " << pid << "\n";
            return 1;
        }
        
        std::cout << "Loading Python " << py_major << "." << py_minor << " ABI...\n";
        auto abi = serpent::create_abi_for_version(py_major, py_minor, reader.get());
        
        if (!abi) {
            std::cerr << "Error: Failed to load ABI plugin for Python " << py_major << "." << py_minor << "\n";
            reader->detach();
            return 1;
        }
        
        std::cout << "Analyzing object at address 0x" << std::hex << addr << "...\n";
        std::cout << "Type name: " << abi->type_name(addr) << "\n";
        
        // For reference: In the future, memory region information can also be displayed as follows
        std::cout << "\nMemory regions:\n";
        auto regions = reader->regions();
        for (size_t i = 0; i < std::min(regions.size(), size_t(10)); ++i) {
            std::cout << std::hex << "0x" << regions[i].start << " - 0x" << regions[i].end 
                      << " [" 
                      << (regions[i].prot & 1 ? "r" : "-") 
                      << (regions[i].prot & 2 ? "w" : "-") 
                      << (regions[i].prot & 4 ? "x" : "-") 
                      << "]\n";
        }
        
        if (regions.size() > 10) {
            std::cout << "... (and " << regions.size() - 10 << " more regions)\n";
        }
        
        reader->detach();
        std::cout << "Detached from process\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
