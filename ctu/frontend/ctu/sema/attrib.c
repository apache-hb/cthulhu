#include "attrib.h"
#include "expr.h"
#include "type.h"

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

    // TODO: this only works for C
    dst->visibility = ENTRYPOINT;
    dst->mangle = "main";
}

static void mangle_attrib(reports_t *reports, attrib_t *dst, vector_t *args) {
    UNUSED(reports);

    lir_t *first = vector_get(args, 0);
    dst->mangle = first->str;
}

static void section_attrib(reports_t *reports, attrib_t *dst, vector_t *args) {
    UNUSED(reports);

    lir_t *first = vector_get(args, 0);
    dst->section = first->str;
}

static void apply_attrib(sema_t *sema, attrib_t *dst, ctu_t *attrib) {
    vector_t *args = attrib->params;
    size_t len = vector_len(args);
    vector_t *result = vector_of(len); /* vector_t<lir_t*> */

    for (size_t i = 0; i < len; i++) {
        ctu_t *expr = vector_get(args, i);
        lir_t *lir = compile_expr(sema, expr);
        vector_set(result, i, lir);
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
        lir_t *cast = implicit_convert_expr(sema, lir, param);

        if (cast == NULL) {
            report(sema->reports, ERROR, attrib->node, "invalid parameter at %zu. found `%s` expected `%s`", i, ctu_type_format(type), ctu_type_format(param));
            error = true;
        }
    }

    if (error) {
        return;
    }

    builtin->callback(sema->reports, dst, result);
}

void init_attribs(void) {
    type_t *voidpfn = type_closure(vector_new(0), type_void());
    type_t *strpfn = type_closure(vector_init(type_string()), type_void());
    builtins = map_new(MAP_SMALL);
    add_builtin("entry", voidpfn, entry_attrib);
    add_builtin("mangle", strpfn, mangle_attrib);
    add_builtin("section", strpfn, section_attrib);
}

void compile_attribs(sema_t *sema, lir_t *lir, ctu_t *ctu) {
    vector_t *attribs = ctu->attribs;

    attrib_t *attr = ctu_malloc(sizeof(attrib_t));
    attr->visibility = ctu->exported ? PUBLIC : PRIVATE;
    attr->mangle = NULL;
    attr->section = NULL;

    for (size_t i = 0; i < vector_len(attribs); i++) {
        apply_attrib(sema, attr, vector_get(attribs, i));
    }

    lir_attribs(lir, attr);
}
