#pragma once

#include "ctu/ast/ast.h"

extern type_t *VOID_TYPE;

void typecheck(nodes_t *nodes);

void sema_init(void);
