#include "pool.hpp"

namespace cthulhu {
    const str* StringPool::intern(const str& string) {
        return &*data.insert(string).first;
    }
}
