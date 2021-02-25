#pragma once

#include <tinyutf8/tinyutf8.h>
#include <vector>

namespace cthulhu {
    using namespace std;

    namespace utf8 = tiny_utf8;

    struct Printer {
        int depth = 0;
        utf8::string buffer = u8"";

        template<typename F>
        void enter(F&& func) {
            depth++;
            func();
            depth--;
        }

        void write(utf8::string str) {
            buffer += utf8::string(depth * 2, ' ');
            buffer += str;
            buffer += "\n";
        }
    };
}
