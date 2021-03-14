#include "lexer.hpp"

namespace cthulhu {
    std::span<char32_t> Lexer::slice(size_t offset, size_t length) const {
        return text.subspan(offset, length);
    }
}
