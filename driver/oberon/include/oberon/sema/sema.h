#pragma once

#include "oberon/ast.h"

#include "cthulhu/hlir/query.h"

typedef enum obr_tags_t {
    eTagValues = eSema2Values,
    eTagTypes = eSema2Types,
    eTagProcs = eSema2Procs,
    eTagModules = eSema2Modules,

    eTagTotal
} obr_tags_t;

h2_t *obr_get_type(h2_t *sema, const char *name);
h2_t *obr_get_module(h2_t *sema, const char *name);

void obr_add_decl(h2_t *sema, obr_tags_t tag, const char *name, h2_t *decl);
