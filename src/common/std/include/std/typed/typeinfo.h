#pragma once

#include "core/compiler.h"

BEGIN_API

typedef size_t (*type_hash_t)(const void *key);
typedef int (*type_cmp_t)(const void *lhs, const void *rhs);

typedef struct typeinfo_t
{
    const char *name;
    size_t size;

    type_hash_t fn_hash;
    type_cmp_t fn_cmp;
} typeinfo_t;

size_t type_hash(const typeinfo_t *type, const void *key);
int type_cmp(const typeinfo_t *type, const void *lhs, const void *rhs);

END_API
