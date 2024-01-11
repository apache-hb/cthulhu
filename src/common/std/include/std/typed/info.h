#pragma once

#include "core/compiler.h"

#include <stdbool.h>
#include <stddef.h>

BEGIN_API

typedef size_t (*hash_fn_t)(const void *key);
typedef bool (*equals_fn_t)(const void *lhs, const void *rhs);

typedef struct type_info_t
{
    hash_fn_t hash;
    equals_fn_t equals;
} type_info_t;

extern const type_info_t kTypeInfoString;
extern const type_info_t kTypeInfoPtr;

END_API
