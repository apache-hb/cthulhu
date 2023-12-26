#pragma once

#include "cthulhu/runtime/common.h"

#include "cthulhu/tree/tree.h"

typedef enum pl0_tag_t {
    ePl0TagValues = eSemaValues,
    ePl0TagTypes = eSemaTypes,
    ePl0TagProcs = eSemaProcs,
    ePl0TagModules = eSemaModules,

    ePl0TagImportedValues,
    ePl0TagImportedProcs,

    ePl0TagTotal
} pl0_tag_t;

void pl0_init(driver_t *handle);

void pl0_forward_decls(context_t *context);
void pl0_process_imports(context_t *context);
void pl0_compile_module(context_t *context);
