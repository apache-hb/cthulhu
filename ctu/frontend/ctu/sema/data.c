#include "data.h"

#include "ctu/util/report-ext.h"

#include "ctu/driver/include.h"

static local_t new_local(void) {
    local_t local = {
        .locals = vector_new(8),
        .result = type_poison("uninitialized return type")
    };

    return local;
}

static stack_t *stack_new(ctu_t *tree) {
    stack_t *stack = ctu_malloc(sizeof(stack_t));
    stack->stack = vector_new(16);

    stack->local = new_local();
    stack->complete = NULL;
    stack->tree = tree;

    stack->externs = vector_new(8);
    stack->lambdas = vector_new(8);
    return stack;
}

void stack_delete(stack_t *stack) {
    ctu_free(stack);
}

state_t *state_new(sema_t *sema, ctu_t *ctu) {
    state_t *state = ctu_malloc(sizeof(state_t));
    state->sema = sema;
    state->ctu = ctu;
    return state;
}

bool stack_enter(sema_t *sema, lir_t *lir) {
    stack_t *stack = sema_get_data(sema);

    size_t len = vector_len(stack->stack);
    for (size_t i = 0; i < len; i++) {
        lir_t *it = vector_get(stack->stack, i);
        if (it == lir) {
            report_recursive(sema->reports, stack->stack, vector_get(stack->stack, 0));
            return false;
        }
    }

    vector_push(&stack->stack, lir);
    return true;
}

void stack_leave(sema_t *sema, lir_t *lir) {
    stack_t *stack = sema_get_data(sema);
    lir_t *elem = vector_pop(stack->stack);

    if (elem != lir) {
        ctu_assert(sema->reports, "parse stack corrupted");
    }
}

void add_var(sema_t *sema, const char *name, lir_t *lir) {
    lir_t *var = get_var(sema, name);
    if (var != NULL) {
        report_shadow(sema->reports, name, var->node, lir->node);
    }

    sema_set(sema, TAG_GLOBALS, name, lir);
}

void add_func(sema_t *sema, const char *name, lir_t *lir) {
    lir_t *func = get_func(sema, name);
    if (func != NULL) {
        report_shadow(sema->reports, name, func->node, lir->node);
    }

    sema_set(sema, TAG_FUNCS, name, lir);
}

void add_type(sema_t *sema, const char *name, type_t *type) {
    const type_t *ty = get_type(sema, name);
    if (ty != NULL) {
        report(sema->reports, ERROR, NULL, "duplicate definition of type `%s`", type_format(type));
    }

    sema_set(sema, TAG_TYPES, name, type);
}

void set_module(sema_t *sema, const char *name, sema_t *mod) {
    sema_set(sema, TAG_IMPORTS, name, mod);
}

lir_t *get_var(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_GLOBALS, name);
}

lir_t *get_func(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_FUNCS, name);
}

type_t *get_type(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_TYPES, name);
}

sema_t *get_module(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_IMPORTS, name);
}

sema_t *new_sema(reports_t *reports, sema_t *parent, size_t *sizes) {
    sema_t *sema = sema_new(parent, parent->path, reports, TAG_MAX, sizes);
    stack_t *stack;
    
    if (parent == NULL) {
        ctu_assert(reports, "new-sema parent == NULL");
        stack = stack_new(NULL);
    } else {
        stack = sema_get_data(parent);
    }

    sema_set_data(sema, stack);
    return sema;
}

static const char *digit_name(sign_t sign, int_t width) {
    switch (width) {
    case TY_CHAR: return sign == SIGNED ? "char": "uchar";
    case TY_SHORT: return sign == SIGNED ? "short": "ushort";
    case TY_INT: return sign == SIGNED ? "int": "uint";
    case TY_LONG: return sign == SIGNED ? "long": "ulong";
    case TY_SIZE: return sign == SIGNED ? "isize": "usize";
    case TY_INTPTR: return sign == SIGNED ? "intptr": "uintptr";
    case TY_INTMAX: return sign == SIGNED ? "intmax": "uintmax";
    default: return "???";
    }
}

sema_t *base_sema(reports_t *reports, ctu_t *tree, size_t decls, size_t imports) {    
    size_t sizes[TAG_MAX] = {
        [TAG_TYPES] = decls,
        [TAG_GLOBALS] = decls,
        [TAG_FUNCS] = decls,
        [TAG_IMPORTS] = imports
    };

    path_t *path = NODE_PATH(tree);

    sema_t *sema = sema_new(NULL, path, reports, TAG_MAX, sizes);
    stack_t *data = stack_new(tree);
    sema_set_data(sema, data);

    for (sign_t sign = 0; sign < SIGN_TOTAL; sign++) {
        for (int_t width = 0; width < TY_INT_TOTAL; width++) {
            const char *name = digit_name(sign, width);
            type_t *type = type_digit_with_name(name, sign, width);
            data->digits[width][sign] = type;
            add_type(sema, name, type);
        }
    }

    add_type(sema, "void", type_void());
    add_type(sema, "bool", type_bool_with_name("bool"));
    add_type(sema, "str", type_string_with_name("str"));

    set_cache(path_realpath(path), sema);
    return sema;
}

void delete_sema(sema_t *sema) {
    sema_delete(sema);
}

void set_return(sema_t *sema, const type_t *type) {
    stack_t *data = sema_get_data(sema);
    data->local.result = type;
}

const type_t *get_return(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    return data->local.result;
}

void add_local(sema_t *sema, lir_t *lir) {
    stack_t *data = sema_get_data(sema);
    vector_push(&data->local.locals, lir);
}

vector_t *move_locals(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    vector_t *locals = data->local.locals;
    data->local.locals = vector_new(16);
    return locals;
}

void add_extern(sema_t *sema, lir_t *lir) {
    stack_t *data = sema_get_data(sema);
    vector_push(&data->externs, lir);
}

vector_t *move_externs(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    vector_t *externs = data->externs;
    data->externs = vector_new(16);
    return externs;
}

void add_lambda(sema_t *sema, lir_t *lir) {
    stack_t *data = sema_get_data(sema);
    vector_push(&data->lambdas, lir);
}

vector_t *move_lambdas(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    vector_t *lambdas = data->lambdas;
    return lambdas;
}

local_t move_state(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    local_t local = data->local;
    data->local = new_local();
    return local;
}

void set_state(sema_t *sema, local_t state) {
    stack_t *data = sema_get_data(sema);
    data->local = state;
}

bool is_discard(const char *name) {
    return startswith(name, "$");
}

type_t *get_cached_digit_type(sema_t *sema, sign_t sign, int_t width) {
    stack_t *data = sema_get_data(sema);
    return data->digits[width][sign];
}


bool is_complete(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    return data->complete != NULL;
}

void make_complete(sema_t *sema, lir_t *lir) {
    stack_t *data = sema_get_data(sema);
    data->complete = lir;
}

lir_t *cached_lir(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    return data->complete;
}

ctu_t *get_tree(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    return data->tree;
}
