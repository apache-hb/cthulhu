#pragma once

namespace ctu
{
    enum class Keyword
    {
#define KEYWORD(id, str) id,
#define OPERATOR(id, str) id,
#define ASM_KEYWORD(id, str) id,
#define RES_KEYWORD(id, str) id,

#include "keywords.inc"
    };
}