#pragma once

#include <gmp.h>

#include "ctu/ast/ast.h"

bool eval_ast(mpz_t result, node_t *ast);
