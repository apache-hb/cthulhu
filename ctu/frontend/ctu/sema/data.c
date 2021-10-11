#include "data.h"

#include "ctu/util/report-ext.h"

stack_t *stack_new(void) {
    stack_t *stack = ctu_malloc(sizeof(stack_t));
    stack->stack = vector_new(16);
    stack->locals = vector_new(8);
    stack->result = type_poison("uninitialized return type");
    stack->externs = vector_new(8);
    return stack;
}

void stack_delete(stack_t *stack) {
    ctu_free(stack, sizeof(stack_t));
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

lir_t *get_var(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_GLOBALS, name);
}

lir_t *get_func(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_FUNCS, name);
}

type_t *get_type(sema_t *sema, const char *name) {
    return sema_get(sema, TAG_TYPES, name);
}

sema_t *new_sema(reports_t *reports, sema_t *parent, size_t *sizes) {
    sema_t *sema = sema_new(parent, reports, TAG_MAX, sizes);
    stack_t *stack;
    
    if (parent == NULL) {
        ctu_assert(reports, "new-sema parent == NULL");
        stack = stack_new();
    } else {
        stack = sema_get_data(parent);
    }

    sema_set_data(sema, stack);
    return sema;
}

sema_t *base_sema(reports_t *reports, size_t decls) {
    size_t sizes[TAG_MAX] = {
        [TAG_TYPES] = decls,
        [TAG_GLOBALS] = decls,
        [TAG_FUNCS] = decls
    };

    sema_t *sema = sema_new(NULL, reports, TAG_MAX, sizes);
    sema_set_data(sema, stack_new());

    add_type(sema, "char", type_digit(SIGNED, TY_CHAR));
    add_type(sema, "uchar", type_digit(UNSIGNED, TY_CHAR));
    add_type(sema, "int", type_digit(SIGNED, TY_INT));
    add_type(sema, "uint", type_digit(UNSIGNED, TY_INT));
    add_type(sema, "isize", type_digit(SIGNED, TY_SIZE));
    add_type(sema, "usize", type_digit(UNSIGNED, TY_SIZE));
    add_type(sema, "void", type_void());
    add_type(sema, "bool", type_bool_with_name("bool"));
    add_type(sema, "str", type_string_with_name("str"));

    return sema;
}

void delete_sema(sema_t *sema) {
    sema_delete(sema);
}

void set_return(sema_t *sema, const type_t *type) {
    stack_t *data = sema_get_data(sema);
    data->result = type;
}

const type_t *get_return(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    return data->result;
}

void add_local(sema_t *sema, lir_t *lir) {
    stack_t *data = sema_get_data(sema);
    vector_push(&data->locals, lir);
}

vector_t *move_locals(sema_t *sema) {
    stack_t *data = sema_get_data(sema);
    vector_t *locals = data->locals;
    data->locals = vector_new(16);
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

bool is_discard(const char *name) {
    return startswith(name, "$");
}
