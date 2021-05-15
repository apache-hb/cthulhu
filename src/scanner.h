#pragma once

#include "ast.h"

typedef struct {
    const char *path;
    node_t *ast;
} scanner_t;
