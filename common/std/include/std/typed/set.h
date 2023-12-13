#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdbool.h>

BEGIN_API

typedef struct typeset_t typeset_t;

typedef bool (*type_equals_t)(const void *lhs, const void *rhs);
typedef size_t (*type_hash_t)(const void *value);

typedef struct typeinfo_t
{
    FIELD_RANGE(>, 0) size_t size;

    type_equals_t pfn_equals;
    type_hash_t pfn_hash;
    const void *empty_value;
} typeinfo_t;

typeset_t *typeset_new(const typeinfo_t *info, size_t len);

const void *typeset_add(typeset_t *set, const void *value);

bool typeset_contains(typeset_t *set, const void *value);

void typeset_reset(typeset_t *set);

END_API
