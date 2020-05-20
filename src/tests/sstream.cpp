#include "compiler/lexer.h"

const char* str = "include x::y::z -> j";

int main() {
    Stream sstream = new StringStream(str);

    std::string buf;
    char c = sstream.peek();
    while((c = sstream.next())) {
        buf += c;
    }

    if(buf != str) {
        return 1;
    }

    return 0;
}