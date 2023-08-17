#pragma once

#include "ctu/ast.h"

#include "cthulhu/hlir/h2.h"

typedef struct lifetime_t lifetime_t;

typedef enum ctu_tag_t {
    eCtuTagValues = eSema2Values,
    eCtuTagTypes = eSema2Types,
    eCtuTagFunctions = eSema2Procs,
    eCtuTagModules = eSema2Modules,

    eCtuTagImports,
    eCtuTagAttribs,
    eCtuTagSuffixes,

    eCtuTagTotal
} ctu_tag_t;

// getting decls

h2_t *ctu_get_namespace(h2_t *sema, const char *name);
h2_t *ctu_get_type(h2_t *sema, const char *name);
h2_t *ctu_get_decl(h2_t *sema, const char *name);

// adding decls

void ctu_add_decl(h2_t *sema, ctu_tag_t tag, const char *name, h2_t *decl);

// runtime module

h2_t *ctu_get_int_type(digit_t digit, sign_t sign);
h2_t *ctu_get_bool_type(void);

vector_t *ctu_rt_path(void);
h2_t *ctu_rt_mod(lifetime_t *lifetime);
