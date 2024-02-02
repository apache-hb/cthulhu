#pragma once

#include <ctu_json_api.h>

#include "core/analyze.h"
#include "core/text.h"

#include <stdbool.h>
#include <gmp.h>

typedef struct io_t io_t;
typedef struct logger_t logger_t;
typedef struct arena_t arena_t;
typedef struct json_t json_t;
typedef struct node_t node_t;
typedef struct map_t map_t;
typedef struct vector_t vector_t;

CT_BEGIN_API

typedef enum json_kind_t
{
#define JSON_TYPE(id, str) id,
#include "json/json.def"

    eJsonCount
} json_kind_t;

typedef struct json_t
{
    json_kind_t kind;
    const node_t *node;

    union {
        /* eJsonString */
        text_t string;

        /* eJsonInteger */
        mpz_t integer;

        /* eJsonFloat */
        float real;

        /* eJsonBoolean */
        bool boolean;

        /* eJsonArray */
        vector_t *array;

        /* eJsonObject */
        map_t *object;

        /* eJsonNull */
        /* empty */
    };
} json_t;

RET_INSPECT
CT_JSON_API json_t *json_scan(IN_NOTNULL io_t *io, IN_NOTNULL logger_t *logger, IN_NOTNULL arena_t *arena);

CT_JSON_API
const char *json_kind_name(IN_RANGE(<, eJsonCount) json_kind_t kind);

CT_END_API
