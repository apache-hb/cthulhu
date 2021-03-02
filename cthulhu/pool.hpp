#pragma once

#include "fwd.hpp"

#include <unordered_set>

namespace cthulhu {
    struct StringPool {
        const utf8::string* intern(const utf8::string& string);
    private:
        std::unordered_set<utf8::string> data;
    };
}
