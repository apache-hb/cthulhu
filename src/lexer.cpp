#include "cthulhu.h"

#include <cctype>
#include <charconv>

namespace cthulhu {
    lexer::lexer(stream::handle* handle)
        : source(handle)
    { }

    char lexer::next() {
        return source.next();
    }

    char lexer::peek() {
        return source.peek();
    }

    char lexer::skip() {
        char c = next();
        while (std::isspace(c))
            c = next();
        return c;
    }

    token lexer::read() {
        char c = skip();
        if (std::isdigit(c)) {
            auto str = collect(c, [](char c) -> bool { return std::isdigit(c); });
            uint64_t n;
            std::from_chars(str.data(), str.data() + str.size(), n);
            return { token::DIGIT, { .digit = n } };
        } else if (c == '-') {
            return { token::OP, { .op = key::SUB } };
        } else if (c == '+') {
            return { token::OP, { .op = key::ADD } };
        } else {
            return { token::END };
        }
    }

    std::string lexer::collect(char c, bool(*filter)(char)) {
        std::string out = {c};
        while (filter(peek()))
            out += next();
        return out;
    }
}