#pragma once

#include "ctu/ast/ast.h"

typedef struct {
    bool case_sensitive;
} traits_t;

void sema_program(traits_t *traits, node_t *program);
