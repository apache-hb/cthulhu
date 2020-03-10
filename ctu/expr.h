#pragma once

#include "keywords.h"

namespace ctu
{
    struct Expr
    {

    };

    struct Unary : Expr
    {
        Keyword op;

        Expr expr;
    };

    struct Binary : Expr
    {
        Keyword op;

        Expr lhs;
        Expr rhs;
    };

    struct Ternary : Expr
    {
        Expr condition;

        Expr truthy;
        Expr falsey;
    };

    struct Constant : Expr
    {

    };
}