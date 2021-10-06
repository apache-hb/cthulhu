#pragma once

#include "ctu/lir/lir.h"
#include "ctu/lir/sema.h"
#include "ctu/frontend/ctu/ast.h"

typedef enum {
    TAG_TYPES,
    TAG_GLOBALS,
    TAG_FUNCS,

    TAG_MAX
} tag_t;

typedef struct {
    vector_t *stack;
    vector_t *locals;
} stack_t;

typedef struct {
    sema_t *sema;
    ctu_t *ctu;
} state_t;

state_t *state_new(sema_t *sema, ctu_t *ctu);

stack_t *stack_new(void);
void stack_delete(stack_t *stack);

bool stack_enter(sema_t *sema, lir_t *lir);
void stack_leave(sema_t *sema, lir_t *lir);

void add_local(sema_t *sema, lir_t *lir);
vector_t *move_locals(sema_t *sema);

void add_var(sema_t *sema, const char *name, lir_t *lir);
void add_func(sema_t *sema, const char *name, lir_t *lir);
void add_type(sema_t *sema, const char *name, type_t *type);

lir_t *get_var(sema_t *sema, const char *name);
lir_t *get_func(sema_t *sema, const char *name);
type_t *get_type(sema_t *sema, const char *name);

sema_t *new_sema(reports_t *reports, sema_t *parent, size_t *sizes);
sema_t *base_sema(reports_t *reports, size_t decls);
void delete_sema(sema_t *sema);
