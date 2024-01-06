#include "std/typed/typeinfo.h"
#include "base/panic.h"

size_t type_hash(const typeinfo_t *type, const void *key)
{
    CTASSERT(type != NULL);
    CTASSERT(type->fn_hash != NULL);

    return type->fn_hash(key);
}

int type_cmp(const typeinfo_t *type, const void *lhs, const void *rhs)
{
    CTASSERT(type != NULL);
    CTASSERT(type->fn_cmp != NULL);

    return type->fn_cmp(lhs, rhs);
}
