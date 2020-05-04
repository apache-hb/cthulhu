#pragma once

#include "ast.h"

struct Parser {
    Stmt next() {
        return Stmt{};
    }

    Program program() {
        return Program{};
    }
};