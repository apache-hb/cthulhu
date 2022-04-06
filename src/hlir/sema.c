#include "cthulhu/hlir/sema.h"
#include "cthulhu/hlir/query.h"

sema_t *sema_new(sema_t *parent, reports_t *reports, size_t decls, size_t *sizes) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    
    sema->parent = parent;
    sema->reports = reports;
    
    sema->decls = vector_of(decls);
    for (size_t i = 0; i < decls; i++) {
        map_t *map = optimal_map(sizes[i]);
        vector_set(sema->decls, i, map);
    }
    
    return sema;
}

void sema_delete(sema_t *sema) {
    ctu_free(sema);
}

void sema_set_data(sema_t *sema, void *data) {
    sema->data = data;
}

void *sema_get_data(sema_t *sema) {
    return sema->data;
}

void sema_set(sema_t *sema, size_t tag, const char *name, void *data) {
    map_t *map = sema_tag(sema, tag);
    map_set(map, name, data);
}

typedef struct {
    hlir_t *result;
    size_t depth;
} sema_query_t;

static sema_query_t sema_inner_get(sema_t *sema, size_t tag, const char *name) {
    map_t *map = sema_tag(sema, tag);

    hlir_t *hlir = map_get(map, name);
    if (hlir != NULL) {
        sema_query_t result = { hlir, 0 };
        return result;
    }

    if (sema->parent != NULL) {
        sema_query_t result = sema_inner_get(sema->parent, tag, name);
        result.depth += 1;
        return result;
    }

    sema_query_t result = { NULL, 0 };
    return result;
}

void *sema_get(sema_t *sema, size_t tag, const char *name) {
    return sema_inner_get(sema, tag, name).result;
}

void *sema_get_with_depth(sema_t *sema, size_t tag, const char *name, size_t *depth) {
    sema_query_t result = sema_inner_get(sema, tag, name);
    *depth = result.depth;
    return result.result;
}

map_t *sema_tag(sema_t *sema, size_t tag) {
    return vector_get(sema->decls, tag);
}

static void report_recursion(reports_t *reports, vector_t *stack, const char *msg) {
    hlir_t *top = vector_tail(stack);
    message_t *id = report(reports, ERROR, top->node, "%s", msg);
    for (size_t i = 0; i < vector_len(stack); i++) {
        hlir_t *hlir = vector_get(stack, i);
        report_append(id, hlir->node, "trace `%zu`", i);
    }
}

static bool find_recursion(reports_t *reports, vector_t **vec, const hlir_t *hlir, const char *msg) {
    vector_t *stack = *vec;
    for (size_t i = 0; i < vector_len(stack); i++) {
        hlir_t *item = vector_get(stack, i);
        if (item != hlir) { continue; }

        report_recursion(reports, stack, msg);
        return true;
    }

    return false;
}

static void check_recursion(reports_t *reports, vector_t **stack, const hlir_t *hlir) {
    if (hlir == NULL) { return; }
    if (find_recursion(reports, stack, hlir, "recursive variable computation")) { return; }

    vector_push(stack, (hlir_t*)hlir);

    switch (hlir->type) {
    case HLIR_NAME:
        check_recursion(reports, stack, hlir->read);
        break;
    case HLIR_GLOBAL: case HLIR_LOCAL:
        check_recursion(reports, stack, hlir->value);
        break;
    case HLIR_BINARY: case HLIR_COMPARE:
        check_recursion(reports, stack, hlir->lhs);
        check_recursion(reports, stack, hlir->rhs);
        break;

    case HLIR_DIGIT_LITERAL:
    case HLIR_BOOL_LITERAL:
    case HLIR_STRING_LITERAL:
        break;

    default:
        ctu_assert(reports, "check-recursion unexpected hlir type %s", hlir_kind_to_string(get_hlir_kind(hlir)));
        break;
    }

    vector_drop(stack);
}

typedef struct {
    const hlir_t *hlir;
    bool nesting;
} entry_t;

static entry_t *new_entry(const hlir_t *hlir, bool nesting) {
    entry_t *entry = ctu_malloc(sizeof(entry_t));
    entry->hlir = hlir;
    entry->nesting = nesting;
    return entry;
}

static void report_type_recursion(reports_t *reports, vector_t *stack) {
    entry_t *top = vector_tail(stack);
    message_t *id = report(reports, ERROR, top->hlir->node, "%s", "recursive type definition");
    for (size_t i = 0; i < vector_len(stack); i++) {
        entry_t *entry = vector_get(stack, i);
        report_append(id, entry->hlir->node, "trace `%zu`", i);
    }
}

static bool find_type_recursion(reports_t *reports, vector_t **vec, const hlir_t *hlir, bool nesting, bool opaque) {
    vector_t *stack = *vec;
    for (size_t i = 0; i < vector_len(stack); i++) {
        entry_t *item = vector_get(stack, i);
        if (item->hlir == hlir) { 
            if (item->nesting && opaque) {
                break;
            }

            report_type_recursion(reports, stack);
            return false;
        }
    }

    vector_push(vec, new_entry(hlir, nesting));

    return true;
}

#define DEPTH_LIMIT 128 // this is what a release deadline looks like :^)

static const hlir_t *chase(reports_t *reports, const hlir_t *hlir) {
    size_t depth = 0;

    while (true) {
        switch (get_hlir_kind(hlir)) {
        case HLIR_POINTER: hlir = hlir->ptr; break;
        case HLIR_ALIAS: hlir = hlir->alias; break;
        case HLIR_FIELD: hlir = get_hlir_type(hlir); break;
        default: return hlir;
        }

        if (depth++ > DEPTH_LIMIT) {
            message_t *id = report(reports, ERROR, hlir->node, "type definition recurses too deep");
            report_note(id, "type definition recurses beyond %d levels", DEPTH_LIMIT);
            return NULL;
        }
    }
}

static void check_type_recursion(reports_t *reports, vector_t **stack, const hlir_t *hlir) {
    if (hlir == NULL) { return; }

    switch (hlir->type) {
    case HLIR_POINTER:
        find_type_recursion(reports, stack, chase(reports, hlir), false, true);
        break;

    case HLIR_CLOSURE: case HLIR_FUNCTION:
        if (find_type_recursion(reports, stack, hlir, false, true)) {
            check_type_recursion(reports, stack, hlir->result);
            for (size_t i = 0; i < vector_len(hlir->params); i++) {
                hlir_t *param = vector_get(hlir->params, i);
                check_type_recursion(reports, stack, param);
            }
        }
        break;

    case HLIR_STRUCT: case HLIR_UNION:
        if (find_type_recursion(reports, stack, hlir, true, false)) {
            for (size_t i = 0; i < vector_len(hlir->fields); i++) {
                hlir_t *field = vector_get(hlir->fields, i);
                check_type_recursion(reports, stack, field);
            }
        }
        break;
    
    case HLIR_ALIAS:
        if (find_type_recursion(reports, stack, hlir, false, false)) {
            check_type_recursion(reports, stack, hlir->alias);
        }
        break;

    case HLIR_FIELD:
        check_type_recursion(reports, stack, get_hlir_type(hlir));
        break;

    case HLIR_DIGIT:
    case HLIR_BOOL:
    case HLIR_STRING:
    case HLIR_VOID:
        break;

    default:
        ctu_assert(reports, "check-type-recursion unexpected hlir type %s", hlir_kind_to_string(get_hlir_kind(hlir)));
        break;
    }

    vector_drop(stack);
}

void check_module(reports_t *reports, hlir_t *mod) {
    size_t nvars = vector_len(mod->globals);
    size_t ntypes = vector_len(mod->types);
    vector_t *vec = vector_new(16);

    for (size_t i = 0; i < ntypes; i++) {
        hlir_t *type = vector_get(mod->types, i);
        check_type_recursion(reports, &vec, type);

        vector_reset(vec);
    }

    for (size_t i = 0; i < nvars; i++) {
        hlir_t *var = vector_get(mod->globals, i);
        CTASSERTF(hlir_is(var, HLIR_GLOBAL), "check-module polluted: global `%s` is %s, not global", get_hlir_name(var), hlir_kind_to_string(get_hlir_kind(var)));
        check_recursion(reports, &vec, var);

        vector_reset(vec);
    }
}
