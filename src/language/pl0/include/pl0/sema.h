// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "cthulhu/tree/tree.h"

typedef struct language_runtime_t language_runtime_t;
typedef struct compile_unit_t compile_unit_t;

typedef enum pl0_tag_t
{
#define DECL_TAG(ID, INIT, STR) ID INIT,
#include "pl0/pl0.inc"
    ePl0TagTotal
} pl0_tag_t;

void pl0_init(language_runtime_t *handle, tree_t *root);

void pl0_forward_decls(language_runtime_t *runtime, compile_unit_t *context);
void pl0_process_imports(language_runtime_t *runtime, compile_unit_t *context);
void pl0_compile_module(language_runtime_t *runtime, compile_unit_t *context);
