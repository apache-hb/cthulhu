#pragma once

#include "ast.h"

#include "cthulhu/hlir/h2.h"

typedef struct map_t map_t;
typedef struct h2_t sema_t;

typedef h2_attrib_t *(*apply_attribs_t)(sema_t *, h2_t *, ast_t *);
typedef bool(*apply_accepts_t)(h2_t *);

typedef struct attrib_t
{
    const char *name;
    h2_kind_t expectedKind;
    apply_attribs_t apply;
    apply_accepts_t accepts;
} attrib_t;

void add_builtin_attribs(sema_t *sema);

void apply_attributes(sema_t *sema, h2_t *hlir, ast_t *ast);
