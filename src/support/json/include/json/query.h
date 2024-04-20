// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "core/analyze.h"

#include "json/json.h"

#include <stdbool.h>

CT_BEGIN_API

typedef struct json_t json_t;
typedef struct logger_t logger_t;
typedef struct arena_t arena_t;

/// @brief query a json object
///
/// @param json the object to query
/// @param query the query to perform
/// @param logger the logger to use
/// @param arena the arena to use
///
/// @return the result of the query
RET_INSPECT
CT_JSON_API json_t *json_query(
    IN_NOTNULL json_t *json,
    IN_NOTNULL const char *query,
    IN_NOTNULL logger_t *logger,
    IN_NOTNULL arena_t *arena);

/// @brief query a json object and ensure it is of a specific type
///
/// @param json the object to query
/// @param query the query to perform
/// @param kind the kind of json value to expect
/// @param logger the logger to use
/// @param arena the arena to use
///
/// @return the result of the query
RET_INSPECT
CT_JSON_API json_t *json_query_type(
    IN_NOTNULL json_t *json,
    IN_NOTNULL const char *query,
    IN_DOMAIN(<, eJsonCount) json_kind_t kind,
    IN_NOTNULL logger_t *logger,
    IN_NOTNULL arena_t *arena);

CT_END_API
