#pragma once

#define UNUSED [[maybe_unused]]

#include <string>

namespace cthulhu {
    enum struct key {
        ADD,
        SUB
    };

    inline std::string debug(key k) {
        switch (k) {
        case key::ADD: return "ADD";
        case key::SUB: return "SUB";
        default: return "ERROR";
        }
    }

    struct stream {
        struct handle {
            virtual ~handle() = default;
            virtual char next() = 0;
        };

        stream(handle* handle);
        char peek();
        char next();

    private:
        handle* source;
        char ahead;
    };

    struct token {
        enum kind { DIGIT, OP, END } type;

        union {
            uint64_t digit;
            key op;
        };

        std::string debug() const {
            switch (type) {
            case DIGIT: return "DIGIT(" + std::to_string(digit) + ")";
            case OP: return "OP(" + cthulhu::debug(op) + ")";
            case END: return "EOF";
            default: return "ERROR";
            }
        }
    };

    struct lexer {
        lexer(stream::handle* handle);

        token read();

    private:
        char next();
        char peek();
        char skip();

        std::string collect(char c, bool(*filter)(char));
        stream source;
    };
}
