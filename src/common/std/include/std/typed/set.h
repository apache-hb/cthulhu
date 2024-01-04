#pragma once

#include "core/compiler.h"

#include <stdbool.h>

BEGIN_API

typedef struct arena_t arena_t;
typedef struct typeinfo_t typeinfo_t;
typedef struct typeset_t typeset_t;

/// @brief create a new typed set
/// @note @p info must contain a @a fn_hash and @a fn_compare function
///
/// @param info the type information for the set
/// @param len the length of the set
/// @param arena the arena to allocate the set from
///
/// @return the new set
typeset_t *typeset_new(const typeinfo_t *info, size_t len, arena_t *arena);

const void *typeset_add(typeset_t *set, const void *value);

bool typeset_contains(typeset_t *set, const void *value);

void typeset_reset(typeset_t *set);

END_API
