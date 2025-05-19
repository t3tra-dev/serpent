#ifdef __linux__
#define _GNU_SOURCE
#include "serpent/core/IProcessReader.h"
#include "serpent/core/MemRegion.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <vector>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace serpent::core {

class LinuxReader final : public IProcessReader {
    int _pid = -1;
    bool _attached = false; // Track attachment status

public:
    ~LinuxReader() override {
        detach();
    }

    bool attach(int pid) override {
        if (_attached) {
            detach(); // Detach if already attached to another process
        }

        if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) == -1) {
            // perror("ptrace PTRACE_ATTACH"); // More informative error
            return false;
        }
        _pid = pid;
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            // perror("waitpid after PTRACE_ATTACH");
            ptrace(PTRACE_DETACH, _pid, nullptr, nullptr);
            _pid = -1;
            return false;
        }

        // Check if the process stopped successfully
        if (WIFSTOPPED(status)) {
            _attached = true;
            return true;
        }

        // If not stopped, something went wrong
        // perror("Process did not stop as expected after PTRACE_ATTACH");
        ptrace(PTRACE_DETACH, _pid, nullptr, nullptr);
        _pid = -1;
        return false;
    }

    void detach() override {
        if (_pid != -1 && _attached) {
            if (ptrace(PTRACE_DETACH, _pid, nullptr, nullptr) == -1) {
                // perror("ptrace PTRACE_DETACH");
            }
        }
        _pid = -1;
        _attached = false;
    }

    bool read(uint64_t addr, void* buf, size_t len) override {
        if (!_attached || _pid == -1) {
            return false;
        }
        iovec local{buf, len};
        iovec remote{reinterpret_cast<void*>(addr), len};
        ssize_t bytes_read = process_vm_readv(_pid, &local, 1, &remote, 1, 0);
        return bytes_read == static_cast<ssize_t>(len);
    }

    std::vector<core::MemRegion> regions() override {
        std::vector<core::MemRegion> result;
        if (!_attached || _pid == -1) {
            return result;
        }

        std::string maps_path = "/proc/" + std::to_string(_pid) + "/maps";
        std::ifstream maps_file(maps_path);
        std::string line;

        if (!maps_file.is_open()) {
            // std::cerr << "Failed to open " << maps_path << std::endl;
            return result;
        }

        while (std::getline(maps_file, line)) {
            std::stringstream ss(line);
            std::string addr_range, perms_str, offset_str, dev_str, inode_str, pathname;
            
            ss >> addr_range >> perms_str >> offset_str >> dev_str >> inode_str;
            // Read the rest of the line as pathname, which might contain spaces
            std::getline(ss >> std::ws, pathname);


            uint64_t start_addr = 0, end_addr = 0;
            size_t dash_pos = addr_range.find('-');
            if (dash_pos != std::string::npos) {
                try {
                    start_addr = std::stoull(addr_range.substr(0, dash_pos), nullptr, 16);
                    end_addr = std::stoull(addr_range.substr(dash_pos + 1), nullptr, 16);
                } catch (const std::exception& e) {
                    // std::cerr << "Error parsing address range: " << addr_range << " - " << e.what() << std::endl;
                    continue; 
                }
            } else {
                // std::cerr << "Invalid address range format: " << addr_range << std::endl;
                continue;
            }


            uint32_t protection_flags = 0;
            if (perms_str.length() >= 1 && perms_str[0] == 'r') protection_flags |= MemRegion::Read;
            if (perms_str.length() >= 2 && perms_str[1] == 'w') protection_flags |= MemRegion::Write;
            if (perms_str.length() >= 3 && perms_str[2] == 'x') protection_flags |= MemRegion::Execute;
            // p (private) or s (shared) - not directly mapped to our flags, but could be stored if needed.

            result.push_back({start_addr, end_addr, end_addr - start_addr, protection_flags, pathname});
        }

        maps_file.close();
        return result;
    }
};

// External C factory (for use with dlopen)
extern "C" serpent::core::IProcessReader* create_reader() {
    return new LinuxReader;
}

} // namespace serpent::core

#else // Not __linux__

#include "serpent/core/IProcessReader.h"
#include "serpent/core/MemRegion.h"
#include <vector>
#include <cstdint>
#include <string>

namespace serpent::core {

class LinuxReader final : public IProcessReader {
public:
    ~LinuxReader() override {}
    bool attach(int pid) override { (void)pid; return false; }
    void detach() override {}
    bool read(uint64_t addr, void* buf, size_t len) override { (void)addr; (void)buf; (void)len; return false; }
    std::vector<core::MemRegion> regions() override { return {}; }
};

extern "C" serpent::core::IProcessReader* create_reader() {
    return nullptr;
}

} // namespace serpent::core

#endif // __linux__
