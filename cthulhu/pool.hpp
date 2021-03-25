#pragma once

#include "fwd.hpp"

#include <unordered_set>

namespace cthulhu {
    struct StringPool {
        const str* intern(const str& string);
        
    private:
        std::unordered_set<str> data;
    };
}
