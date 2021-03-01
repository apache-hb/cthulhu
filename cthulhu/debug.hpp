#pragma once

#include <tinyutf8/tinyutf8.h>
#include <vector>

//
// code for printing the ast for debugging
//

namespace utf8 = tiny_utf8;

namespace cthulhu {
    using namespace std;

    struct Printer {
        int depth = 0;
        utf8::string buffer = u8"";

        template<typename F>
        void enter(F&& func) {
            depth++;
            func();
            depth--;
        }

        template<typename F>
        void section(utf8::string str, F&& func) {
            write(str);
            enter(func);
        }

        void write(utf8::string str) {
            buffer += utf8::string(depth * 2, ' ');
            buffer += str;
            buffer += "\n";
        }
    };
}
