// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_json_api.h>

#include "core/analyze.h"
#include "core/text.h"
#include "core/where.h"
#include "std/typed/vector.h"

#include <gmp.h>

#include <stdbool.h>

typedef struct io_t io_t;
typedef struct logger_t logger_t;
typedef struct arena_t arena_t;
typedef struct json_t json_t;
typedef struct node_t node_t;
typedef struct scan_t scan_t;
typedef struct map_t map_t;
typedef struct vector_t vector_t;

CT_BEGIN_API

/// @defgroup json JSON parsing
/// @brief simple JSON parsing
/// @ingroup support
/// @{

/// @brief the kind of json value
typedef enum json_kind_t
{
#define JSON_TYPE(id, str) id,
#include "json/json.inc"

    eJsonCount
} json_kind_t;

/// @brief a json value
typedef struct json_t
{
    /// @brief the kind of json value
    json_kind_t kind;

    /// @brief the source location of the json value
    /// @note this is only the position of the span, and does not include the
    ///       source file. it is the user's responsibility to track the file.
    where_t where;

    union {
        /// @brief the string value of this node
        /// @warning only valid if @a kind is @a eJsonString
        text_view_t string;

        /// @brief the integer value of this node
        /// @warning only valid if @a kind is @a eJsonInteger
        mpz_t integer;

        /// @brief the float value of this node
        /// @warning only valid if @a kind is @a eJsonFloat
        float real;

        /// @brief the boolean value of this node
        /// @warning only valid if @a kind is @a eJsonBoolean
        bool boolean;

        /// @brief the array value of this node
        /// @warning only valid if @a kind is @a eJsonArray
        typevec_t array;

        /// @brief the object value of this node
        /// @warning only valid if @a kind is @a eJsonObject
        const map_t *object;

        /* eJsonNull */
        /* empty */
    };
} json_t;

typedef struct json_parse_t
{
    scan_t *scanner;
    json_t *root;
} json_parse_t;

/// @brief get a json value from an object by key
///
/// @param json the object to get the value from
/// @param key the key to get the value of
///
/// @return the value of the key
RET_INSPECT
CT_JSON_API json_t *json_map_get(IN_NOTNULL const json_t *json, IN_NOTNULL const char *key);

/// @brief get an array element by index
///
/// @param json the array to get the element from
/// @param index the index of the element to get
///
/// @return the element at the index
RET_INSPECT
CT_JSON_API json_t *json_array_get(IN_NOTNULL const json_t *json, size_t index);

/// @brief scan an io into a json value
/// scan the contents of an io object into a json value
/// @note if the scan fails, the logger will contain error information
///
/// @param io the io to scan
/// @param logger the logger to report errors to
/// @param arena the arena to use
///
/// @retval NULL if the scan failed
/// @retval json_t* the scanned json value
RET_INSPECT
CT_JSON_API json_t *json_scan(IN_NOTNULL io_t *io, IN_NOTNULL logger_t *logger, IN_NOTNULL arena_t *arena);

/// @brief parse an io into a json value
/// parse the contents of an io object into a json value
/// @note if the parse fails, the logger will contain error information
///
/// @param io the io to parse
/// @param logger the logger to report errors to
/// @param arena the arena to use
///
/// @return the parse result
RET_INSPECT
CT_JSON_API json_parse_t json_parse(IN_NOTNULL io_t *io, IN_NOTNULL logger_t *logger, IN_NOTNULL arena_t *arena);

/// @brief pretty print a json value to an io
///
/// @param json the json value to print
/// @param io the io to print to
/// @param indent the number of spaces to indent, or 0 for compact printing
/// @param tabs if true, use tabs instead of spaces
CT_JSON_API void json_print(IN_NOTNULL const json_t *json, IN_NOTNULL io_t *io, size_t indent, bool tabs);

/// @brief get the name of a json kind
///
/// @param kind the kind to get the name of
///
/// @return the name of the kind
RET_NOTNULL
CT_JSON_API const char *json_kind_name(IN_DOMAIN(<, eJsonCount) json_kind_t kind);


/// @}

CT_END_API
