#pragma once

#include "ast.h"
#include "cthulhu/hlir/hlir.h"

typedef struct map_t map_t;
typedef struct sema_t sema_t;

typedef hlir_attributes_t *(*apply_attribs_t)(sema_t *, hlir_t *, ast_t *);
typedef bool(*apply_accepts_t)(hlir_t *);

typedef struct attrib_t
{
    const char *name;
    hlir_kind_t expectedKind;
    apply_attribs_t apply;
    apply_accepts_t accepts;
} attrib_t;

void add_builtin_attribs(sema_t *sema);

void apply_attributes(sema_t *sema, hlir_t *hlir, ast_t *ast);
