#pragma once

#include "cthulhu/hlir/h2.h"

typedef struct driver_t driver_t;
typedef struct context_t context_t;

typedef enum ctu_tag_t {
    eTagValues = eSema2Values,
    eTagTypes = eSema2Types,
    eTagFunctions = eSema2Procs,
    eTagModules = eSema2Modules,

    eTagImports,
    eTagAttribs,
    eTagSuffix,

    eTagTotal
} ctu_tag_t;

void ctu_init(driver_t *handle);

void ctu_forward_decls(context_t *context);
void ctu_process_imports(context_t *context);
void ctu_compile_module(context_t *context);
