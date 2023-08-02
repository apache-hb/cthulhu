#pragma once

#include "cthulhu/hlir/h2.h"

typedef struct context_t context_t;

typedef enum obr_tags_t {
    eTagValues = eSema2Values,
    eTagTypes = eSema2Types,
    eTagProcs = eSema2Procs,
    eTagModules = eSema2Modules,

    eTagTotal
} obr_tags_t;

void obr_forward_decls(context_t *context);
void obr_process_imports(context_t *context);
void obr_compile_module(context_t *context);
