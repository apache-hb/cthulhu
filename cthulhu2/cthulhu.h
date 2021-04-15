#pragma once

#include <peglib.h>
#include <fmt/core.h>

namespace ctu {
    template<typename T = std::runtime_error, typename... A>
    [[noreturn]] void panic(const char* fmt, const A&... args) {
        throw T(fmt::format(fmt, args...));
    }

    struct Context {

    };

    void init();
    Context parse(std::string source);
}