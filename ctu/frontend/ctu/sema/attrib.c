#include "attrib.h"
#include "expr.h"
#include "ctu/type/retype.h"

typedef void(*callback_t)(reports_t*, attrib_t*, vector_t*);

typedef struct {
    type_t *type;
    callback_t callback;
} builtin_t;

static map_t *builtins = NULL;

static void add_builtin(const char *name, type_t *type, callback_t callback) {
    builtin_t *builtin = ctu_malloc(sizeof(builtin_t));

    builtin->type = type;
    builtin->callback = callback;

    map_set(builtins, name, builtin);
}

static void entry_attrib(reports_t *reports, attrib_t *dst, vector_t *args) {
    UNUSED(reports);
    UNUSED(args);

    dst->visibility = ENTRYPOINT;
    dst->mangle = "main";
}

static void mangle_attrib(reports_t *reports, attrib_t *dst, vector_t *args) {
    UNUSED(reports);

    lir_t *first = vector_get(args, 0);
    dst->mangle = first->str;
}

static void apply_attrib(sema_t *sema, attrib_t *dst, ctu_t *attrib) {
    vector_t *args = attrib->params;
    size_t len = vector_len(args);
    vector_t *result = vector_of(len); /* vector_t<lir_t*> */

    for (size_t i = 0; i < len; i++) {
        ctu_t *expr = vector_get(args, i);
        lir_t *lir = compile_expr(sema, expr);
        lir_t *retyped = retype_expr(sema->reports, type_any(), lir);
        vector_set(result, i, retyped);
    }

    builtin_t *builtin = map_get(builtins, attrib->name);
    if (builtin == NULL) {
        report(sema->reports, WARNING, attrib->node, "unknown attribute `%s`", attrib->name);
        return;
    }

    bool error = false;

    for (size_t i = 0; i < len; i++) {
        lir_t *lir = vector_get(result, i);
        const type_t *type = lir_type(lir);
        const type_t *param = param_at(builtin->type, i);

        if (is_poison(types_common(type, param))) {
            report(sema->reports, ERROR, attrib->node, "invalid parameter at %zu. found `%s` expected `%s`", i, type_format(type), type_format(param));
            error = true;
        }
    }

    if (error) {
        return;
    }

    builtin->callback(sema->reports, dst, result);
}

void init_attribs(void) {
    builtins = map_new(MAP_SMALL);
    add_builtin("entry", type_closure(vector_new(0), type_void()), entry_attrib);
    add_builtin("mangle", type_closure(vector_init(type_string()), type_void()), mangle_attrib);
}

void compile_attribs(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    vector_t *attribs = ctu->attribs;

    attrib_t *attr = ctu_malloc(sizeof(attrib_t));
    attr->visibility = ctu->exported ? PUBLIC : PRIVATE;
    attr->mangle = NULL;

    for (size_t i = 0; i < vector_len(attribs); i++) {
        apply_attrib(sema, attr, vector_get(attribs, i));
    }

    lir_attribs(lir, attr);
}
