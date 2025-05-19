#include "serpent/core/IProcessReader.h"
#include "serpent/core/MemRegion.h"
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_vm.h>
#include <libproc.h>
#include <vector>
#include <iostream>

namespace serpent::core {

class MacOSReader final : public IProcessReader {
private:
    int _pid = -1;
    mach_port_t _task = MACH_PORT_NULL;

public:
    ~MacOSReader() {
        detach();
    }

    bool attach(int pid) override {
        kern_return_t kr;
        
        if (_task != MACH_PORT_NULL) {
            detach();
        }
        
        _pid = pid;
        
        // Requires task_for_pid privileges (root privileges or a privileged process)
        kr = task_for_pid(mach_task_self(), pid, &_task);
        if (kr != KERN_SUCCESS) {
            std::cerr << "task_for_pid failed: " << mach_error_string(kr) 
                    << " (Error code: " << kr << ")" << std::endl;
            _task = MACH_PORT_NULL;
            return false;
        }
        
        std::cout << "macOS reader successfully attached" << std::endl;
        return true;
    }

    void detach() override {
        if (_task != MACH_PORT_NULL) {
            mach_port_deallocate(mach_task_self(), _task);
            _task = MACH_PORT_NULL;
        }
        _pid = -1;
    }

    bool read(uint64_t addr, void* buf, size_t len) override {
        if (_task == MACH_PORT_NULL) {
            return false;
        }

        mach_vm_size_t bytes_read = 0;
        kern_return_t kr = mach_vm_read_overwrite(
            _task,
            addr,
            len,
            reinterpret_cast<mach_vm_address_t>(buf),
            &bytes_read);
            
        return (kr == KERN_SUCCESS && bytes_read == len);
    }

    std::vector<core::MemRegion> regions() override {
        std::vector<core::MemRegion> result;
        
        if (_task == MACH_PORT_NULL) {
            return result;
        }
        
        mach_vm_address_t current_address = 0;
        mach_vm_size_t current_size = 0;
        
        while (true) {
            // Retrieve the next memory region
            mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
            vm_region_basic_info_data_64_t region_info;
            mach_port_t object_name = MACH_PORT_NULL;
            
            kern_return_t kr = mach_vm_region(
                _task,
                &current_address, // Use current_address here
                &current_size,    // Use current_size here
                VM_REGION_BASIC_INFO_64,
                reinterpret_cast<vm_region_info_t>(&region_info),
                &count,
                &object_name);
                
            if (kr != KERN_SUCCESS) {
                break; // Reached the end of regions or encountered an error
            }
            
            // Convert protection flags to Linux-style format
            uint32_t protection_flags = 0;
            if (region_info.protection & VM_PROT_READ) protection_flags |= 1;
            if (region_info.protection & VM_PROT_WRITE) protection_flags |= 2;
            if (region_info.protection & VM_PROT_EXECUTE) protection_flags |= 4;
            
            // Use the address and size obtained from mach_vm_region
            result.push_back({current_address, current_address + current_size, current_size, protection_flags, ""});
            
            // Move to the next region
            current_address += current_size;
            
            // Prevent infinite loops as a safeguard
            if (result.size() > 10000) { // Arbitrary limit to prevent runaway loops
                std::cerr << "Warning: Exceeded maximum region count in MacOSReader::regions()" << std::endl;
                break;
            }
        }
        
        return result;
    }
};

// External C factory function (for use with dlopen)
extern "C" IProcessReader* create_reader() {
    return new MacOSReader; 
}

} // namespace serpent::core
