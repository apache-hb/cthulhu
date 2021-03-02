#include "error.hpp"

namespace cthulhu {
    LexerError::LexerError(Lexer* lexer, LexerError::Type type)
        : lexer(lexer)
        , type(type)
    { }

    const char* LexerError::what() const noexcept {
        switch (type) {
        case LexerError::CAST: return "invalid cast";
        case LexerError::CHAR: return "invalid char";
        default: return "unknown";
        }
    }
}
