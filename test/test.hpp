#pragma once

#include <stdlib.h>
#include <stdexcept>

#define ASSERT(expr) if (!(expr)) { throw std::logic_error(#expr); }

template<typename T, typename F>
void except(F&& func) {
    bool threw = false;

    try {
        func();
    } catch (const T& err) {
        threw = true;
    }

    ASSERT(threw);
}
