#pragma once

#include "oberon/ast.h"

#include "cthulhu/hlir/query.h"

typedef struct lifetime_t lifetime_t;

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

h2_t *obr_get_digit_type(digit_t digit, sign_t sign);
h2_t *obr_get_bool_type(void);

h2_t *obr_rt_mod(lifetime_t *lifetime);
vector_t *obr_rt_path(void);
