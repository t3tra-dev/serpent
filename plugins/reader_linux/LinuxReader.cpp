#include "serpent/core/IProcessReader.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <vector>
#include <unistd.h>

#ifdef __linux__
#else
    #define PTRACE_ATTACH 16
    #define PTRACE_DETACH 17
    ssize_t process_vm_readv(pid_t pid, const struct iovec *local_iov,
                             unsigned long liovcnt,
                             const struct iovec *remote_iov,
                             unsigned long riovcnt, unsigned long flags) {
    errno = ENOSYS;
    return -1;
}
#endif

namespace serpent {

class LinuxReader final : public IProcessReader {
    int _pid = -1;
public:
    bool attach(int pid) override {
        if (ptrace(PTRACE_ATTACH, pid, nullptr, NULL) == -1) return false;
        _pid = pid;
        waitpid(pid, nullptr, 0);
        return true;
    }
    void detach() override {
        if (_pid != -1) { ptrace(PTRACE_DETACH, _pid, nullptr, NULL); }
    }
    bool read(uint64_t addr, void* buf, size_t len) override {
        iovec local{buf, len}, remote{reinterpret_cast<void*>(addr), len};
        return process_vm_readv(_pid, &local, 1, &remote, 1, 0) == (ssize_t)len;
    }
    std::vector<MemRegion> regions() override { /* /proc/pid/maps parsing */ return {}; }
};

// External C factory (for use with dlopen)
extern "C" serpent::IProcessReader* create_reader() { return new LinuxReader; }

} // namespace serpent
