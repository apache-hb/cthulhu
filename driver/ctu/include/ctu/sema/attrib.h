#pragma once

typedef struct tree_t tree_t;
typedef struct vector_t vector_t;

typedef void (*ctu_attrib_apply_t)(tree_t *sema, tree_t *decl, vector_t *args);

typedef struct ctu_attrib_t {
    const char *name;
    ctu_attrib_apply_t fnApply;
} ctu_attrib_t;

ctu_attrib_t *ctu_attrib_entry(void);

void ctu_apply_attribs(tree_t *sema, tree_t *decl, vector_t *attribs);
