#pragma once

#include <fmt/format.h>
#include <exception>

namespace ctu {
    template<typename T = std::runtime_error, typename... A>
    [[noreturn]] void panic(const char* fmt, const A&... args) {
        throw T(fmt::format(fmt, args...));
    }
}
