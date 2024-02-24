// SPDX-License-Identifier: GPL-3.0-only

#pragma once

typedef struct tree_t tree_t;
typedef struct vector_t vector_t;
typedef struct arena_t arena_t;

typedef void (*ctu_attrib_apply_t)(tree_t *sema, tree_t *decl, const vector_t *args);

typedef struct ctu_attrib_t {
    const char *name;
    ctu_attrib_apply_t fn_apply;
} ctu_attrib_t;

void ctu_init_attribs(tree_t *sema, arena_t *arena);
void ctu_apply_attribs(tree_t *sema, tree_t *decl, const vector_t *attribs);
