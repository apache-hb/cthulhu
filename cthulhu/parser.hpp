#pragma once

#include "fwd.hpp"
#include "ast.hpp"

namespace cthulhu {
    struct Parser {
    private:
        Lexer* lexer;
        Token ahead;
    };
}
