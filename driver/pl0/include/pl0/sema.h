#pragma once

#include "cthulhu/mediator/driver.h"

#include "cthulhu/tree/tree.h"

typedef enum pl0_tag_t {
    ePl0TagValues = eSema2Values,
    ePl0TagTypes = eSema2Types,
    ePl0TagProcs = eSema2Procs,
    ePl0TagModules = eSema2Modules,

    ePl0TagImportedValues,
    ePl0TagImportedProcs,

    ePl0TagTotal
} pl0_tag_t;

void pl0_init(driver_t *handle);

void pl0_forward_decls(context_t *context);
void pl0_process_imports(context_t *context);
void pl0_compile_module(context_t *context);
