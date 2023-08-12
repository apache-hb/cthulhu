#pragma once

#include "cthulhu/mediator/driver.h"

#include "cthulhu/hlir/h2.h"

typedef enum pl0_tag_t {
    eTagValues = eSema2Values,
    eTagTypes = eSema2Types,
    eTagProcs = eSema2Procs,
    eTagModules = eSema2Modules,

    eTagImportedValues,
    eTagImportedProcs,

    eTagTotal
} pl0_tag_t;

void pl0_init(driver_t *handle);

void pl0_forward_decls(context_t *context);
void pl0_process_imports(context_t *context);
void pl0_compile_module(context_t *context);
