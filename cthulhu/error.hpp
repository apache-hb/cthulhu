#pragma once

#include "fwd.hpp"
#include <exception>

namespace cthulhu {
    struct Error : std::exception {

    };

    struct LexerError final : Error {
        enum Type {
            CHAR, // invalid character
            CAST, // invalid cast from token
            END, // the end of the file was reached when it was not expected
        };

        LexerError(Lexer* lexer, Type type);

        const char* what() const noexcept;
    private:
        Lexer* lexer;
        Type type;
    };

    struct ParserError final : Error {

    };
}
