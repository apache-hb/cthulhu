#include "cthulhu/data/type.h"

#include "cthulhu/util/util.h"

static type_t *type_new(metatype_t meta, const char *name) {
    type_t *type = ctu_malloc(sizeof(type_t));
    type->type = meta;
    type->name = name;
    return type;
}

type_t *type_integer(const char *name) {
    return type_new(TYPE_INTEGER, name);
}

type_t *type_boolean(const char *name) {
    return type_new(TYPE_BOOLEAN, name);
}

type_t *type_error(const char *error) {
    return type_new(TYPE_ERROR, error);
}
