#include "pool.hpp"

namespace cthulhu {
    const utf8::string* StringPool::intern(const utf8::string& string) {
        return &*data.insert(string).first;
    }
}
