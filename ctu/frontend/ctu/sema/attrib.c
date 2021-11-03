#include "attrib.h"
#include "expr.h"
#include "type.h"

typedef void(*callback_t)(reports_t*, attrib_t*, vector_t*);
typedef lir_t*(*detail_t)(sema_t*, type_t*, ctu_t*);

typedef struct {
    type_t *type;
    union {
        callback_t callback;
        detail_t detail;
    };
} builtin_t;

static map_t *builtins = NULL;
static map_t *details = NULL;

static void add_builtin(const char *name, type_t *type, callback_t callback) {
    builtin_t *builtin = ctu_malloc(sizeof(builtin_t));

    builtin->type = type;
    builtin->callback = callback;

    map_set(builtins, name, builtin);
}

static void add_detail(const char *name, detail_t detail) {
    builtin_t *builtin = ctu_malloc(sizeof(builtin_t));
    builtin->detail = detail;
    map_set(details, name, builtin);
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

static void section_attrib(reports_t *reports, attrib_t *dst, vector_t *args) {
    UNUSED(reports);

    lir_t *first = vector_get(args, 0);
    dst->section = first->str;
}

static lir_t *sizeof_detail(sema_t *sema, type_t *type, ctu_t *ctu) {
    lir_t *lir = lir_forward(ctu->node, NULL, LIR_DEFINE, NULL);
    lir_define(sema->reports, lir,
        type_closure(vector_new(0), type_usize()),
        lir_return(ctu->node, lir_detail_sizeof(ctu->node, type))
    );

    add_lambda(sema, lir);

    return lir;
}

static lir_t *alignof_detail(sema_t *sema, type_t *type, ctu_t *ctu) {
    lir_t *lir = lir_forward(ctu->node, NULL, LIR_DEFINE, NULL);
    lir_define(sema->reports, lir,
        type_closure(vector_new(0), type_usize()),
        lir_return(ctu->node, lir_detail_alignof(ctu->node, type))
    );

    add_lambda(sema, lir);

    return lir;
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

    details = map_new(MAP_SMALL);
    add_detail("sizeof", sizeof_detail);
    add_detail("alignof", alignof_detail);
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

lir_t *compile_detail(sema_t *sema, ctu_t *ctu, type_t *type, const char *detail) {
    builtin_t *builtin = map_get(details, detail);
    if (builtin == NULL) {
        report(sema->reports, ERROR, ctu->node, "unknown detail `%s`", detail);
        return lir_poison(ctu->node, "unknown detail");
    }

    return builtin->detail(sema, type, ctu);
}
