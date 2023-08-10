#pragma once

#include "ctu/ast.h"

#include "cthulhu/hlir/h2.h"

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

// getting decls

h2_t *ctu_get_namespace(h2_t *sema, const char *name);
h2_t *ctu_get_type(h2_t *sema, const char *name);

// adding decls

void ctu_add_decl(h2_t *sema, ctu_tag_t tag, const char *name, h2_t *decl);

// runtime module

h2_t *ctu_get_int_type(digit_t digit, sign_t sign);
vector_t *ctu_rt_path(void);
h2_t *ctu_rt_mod(reports_t *reports);
