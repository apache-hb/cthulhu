#include "token.hpp"

namespace cthulhu {
    std::span<char32_t> Token::text() const {
        return lexer->slice(first, last);
    }
}
