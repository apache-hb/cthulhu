#pragma once

#include "cthulhu/tree/tree.h"

typedef struct language_runtime_t language_runtime_t;
typedef struct compile_unit_t compile_unit_t;

typedef enum pl0_tag_t {
    ePl0TagValues = eSemaValues,
    ePl0TagTypes = eSemaTypes,
    ePl0TagProcs = eSemaProcs,
    ePl0TagModules = eSemaModules,

    ePl0TagImportedValues,
    ePl0TagImportedProcs,

    ePl0TagTotal
} pl0_tag_t;

typedef struct pl0_sema_t
{
    tree_t *sema;
    arena_t *arena;
    logger_t *reports;
} pl0_sema_t;

void pl0_init(language_runtime_t *handle, tree_t *root);

void pl0_forward_decls(language_runtime_t *runtime, compile_unit_t *context);
void pl0_process_imports(language_runtime_t *runtime, compile_unit_t *context);
void pl0_compile_module(language_runtime_t *runtime, compile_unit_t *context);
