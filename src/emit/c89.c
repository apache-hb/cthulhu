#include "cthulhu/emit/emit.h"

static char *get_type(const type_t *type);

static char *emit_signature(const type_t *type) {
    const char *args = "void";
    if (vector_len(type->params) > 0) {
        vector_t *params = VECTOR_MAP(type->params, type_get_name);
        args = strjoin(", ", params);
    }
    return format("%s(*type_%s)(%s)", get_type(type->result), type_get_name(type), args);
}

static char *emit_type_decl(const type_t *type) {
    switch (type->type) {
    case TYPE_INTEGER:
        return format("typedef int type_%s;", type->name);
    case TYPE_BOOLEAN:
        return format("typedef bool type_%s;", type->name);
    case TYPE_STRING:
        return format("typedef const char *type_%s;", type->name);
    case TYPE_VOID:
        return format("typedef void type_%s;", type->name);
    case TYPE_SIGNATURE:
        return format("typedef %s;", emit_signature(type));
    default:
        return NULL;
    }
}

static char *get_type(const type_t *type) {
    return format("type_%s", type->name);
}

void c89_emit_tree(reports_t *reports, const hlir_t *hlir) {
    vector_t *types = VECTOR_MAP(hlir->types, emit_type_decl);

    char *joined = strjoin("\n", types);
    printf("%s\n", joined);
}
