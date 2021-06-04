#pragma once

#include "ast.h"

typedef struct {
    const char *path;
    nodes_t *ast;
} scanner_t;
