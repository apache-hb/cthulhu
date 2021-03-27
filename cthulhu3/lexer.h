#pragma once

#include "token.h"

#include <memory>

struct lexer {
    lexer(stream source, std::shared_ptr<pool> pool = std::make_shared<pool>());

    stream source;
    std::shared_ptr<pool> pool;

    std::string text;

    size_t depth = 0;
    size_t start = 0;
    size_t offset = 0;
};
