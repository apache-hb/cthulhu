#pragma once

#include <tinyutf8/tinyutf8.h>
#include <vector>
#include <memory>

namespace utf8 = tiny_utf8;

using c32 = char32_t;

template<typename T>
using vec = std::vector<T>;

template<typename T>
using ptr = std::shared_ptr<T>;

#define MAKE std::make_shared
#define SELF std::dynamic_pointer_cast

namespace cthulhu {
    struct Stream;
    struct Lexer;
    struct Token;
}

namespace cthulhu::ast {
    struct Node;
    struct Type;
    struct Expr;
}
