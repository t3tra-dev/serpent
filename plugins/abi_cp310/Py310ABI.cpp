// *超簡易版: 型名だけ読む*
#include "serpent/core/IPythonABI.h"
#include "serpent/core/IProcessReader.h"

namespace serpent {

struct PyObject310 {
    uint64_t ob_refcnt;
    uint64_t ob_type;   // → PyTypeObject*
};

struct PyTypeObject310 {
    uint64_t ob_base[2];    // PyObject_HEAD
    uint64_t ob_size;
    uint64_t tp_name;       // const char*
    // ... unused beyond this point
};

class Py310ABI final : public IPythonABI {
    IProcessReader* _reader;
public:
    explicit Py310ABI(IProcessReader* r) : _reader(r) {}
    std::string type_name(uint64_t addr) override {
        PyObject310 obj;
        if (!_reader->read(addr, &obj, sizeof(obj))) return "<err>";
        PyTypeObject310 type;
        _reader->read(obj.ob_type, &type, sizeof(type));
        char buf[128]{};
        _reader->read(type.tp_name, buf, sizeof(buf)-1);
        return buf;
    }
    size_t size(uint64_t) override { return 0; }
    std::vector<uint64_t> references(uint64_t) override { return {}; }
};

extern "C" serpent::IPythonABI* create_abi(serpent::IProcessReader* r)
{ return new Py310ABI(r); }

} // namespace serpent
