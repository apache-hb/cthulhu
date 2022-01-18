#pragma once

#include "cthulhu/lir/lir.h"
#include "cthulhu/lir/sema.h"
#include "cthulhu/frontend/ctu/ast.h"

typedef enum {
    TAG_TYPES,
    TAG_USERTYPES,
    TAG_GLOBALS,
    TAG_FUNCS,

    TAG_MAX
} tag_t;

typedef struct {
    const type_t *result; /// the return type of the current function
} local_t;

typedef struct {
    lir_t *complete;
    ctu_t *tree;
    vector_t *stack;
    type_t *digits[TY_INT_TOTAL][SIGN_TOTAL];
    type_t *boolean;

    local_t local; /// function local data
    vector_t *externs; /// all extern functions in the current module
    vector_t *lambdas; /// all lambdas in the current module
} stack_t;

typedef struct {
    sema_t *sema;
    ctu_t *ctu;
} state_t;

state_t *state_new(sema_t *sema, ctu_t *ctu);

void stack_delete(stack_t *stack);

bool stack_enter(sema_t *sema, lir_t *lir);
void stack_leave(sema_t *sema, lir_t *lir);

void set_return(sema_t *sema, const type_t *type);
const type_t *get_return(sema_t *sema);

void add_extern(sema_t *sema, lir_t *lir);
vector_t *move_externs(sema_t *sema);

void add_lambda(sema_t *sema, lir_t *lir);
vector_t *move_lambdas(sema_t *sema);

local_t move_state(sema_t *sema);
void set_state(sema_t *sema, local_t state);

void add_var(sema_t *sema, const char *name, lir_t *lir);
void add_func(sema_t *sema, const char *name, lir_t *lir);
void add_type(sema_t *sema, const char *name, type_t *type);

lir_t *get_var(sema_t *sema, const char *name);
lir_t *get_func(sema_t *sema, const char *name);
type_t *get_type(sema_t *sema, const char *name);

sema_t *new_sema(reports_t *reports, sema_t *parent, size_t *sizes);
sema_t *base_sema(reports_t *reports, ctu_t *tree, size_t decls);
void delete_sema(sema_t *sema);

bool is_discard(const char *name);

type_t *get_cached_digit_type(sema_t *sema, sign_t sign, int_t width);
type_t *get_cached_bool_type(sema_t *sema);
type_t *get_cached_string_type(sema_t *sema);
type_t *get_cached_usize(sema_t *sema);

bool is_complete(sema_t *sema);
void make_complete(sema_t *sema, lir_t *lir);
lir_t *cached_lir(sema_t *sema);

ctu_t *get_tree(sema_t *sema);
