#pragma once

#include <string>
#include <variant>


namespace ct {
    // the type of a token
    enum class TokenType {
        // invalid token
        invalid,

        // end of file
        eof,

        // identifier. abc_123
        ident,

        // string. "string"
        string,

        // single character. 'a'
        character,

        // floating point literal.
        //  - 0.0
        //  - 0.1f
        //  - 123_321.567
        decimal,

        // integer literal
        //  - 100
        //  - 0b11001100 binary literal
        //  - 0x500 hex literal
        //  - 0o777 octal literal
        //  - 100_000 underscores allowed
        //  - 100u8 suffix to coerce type
        integer,

        // keyword
        keyword
    };

    // a position in a source file
    struct SourcePosition {
        // the number of bytes from the start of the file
        int pos;

        // the column of the first char
        int col;

        int line;
    };

    enum class Keyword {

    };

    using TokenData = std::variant<
        std::monostate,
        std::string,
        Keyword
    >;

    struct Token {
        // the type of the token
        TokenType type;

        // position of the token
        SourcePosition pos;

        // the token of the data
        TokenData data;
    };
}