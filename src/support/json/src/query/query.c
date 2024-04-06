// SPDX-License-Identifier: LGPL-3.0-or-later
#include "json/query.h"
#include "json/json.h"

#include "arena/arena.h"
#include "interop/compile.h"

#include "io/io.h"
#include "notify/notify.h"
#include "base/panic.h"

#include "query_scan.h"

#include "query_bison.h" // IWYU pragma: keep
#include "query_flex.h" // IWYU pragma: keep
#include "std/typed/vector.h"
#include "std/vector.h"

CT_CALLBACKS(kQueryCallbacks, query);

static json_t *eval_query(json_t *json, const query_ast_t *query, arena_t *arena)
{
    if (json == NULL)
        return NULL;

    switch (query->kind)
    {
    case eQueryObject:
        return json;
    case eQueryField:
    {
        json_t *object = eval_query(json, query->object, arena);
        if (!object)
            return NULL;

        if (object->kind != eJsonObject)
            return NULL;

        return json_map_get(object, arena_strndup(query->field.text, query->field.length, arena));
    }
    case eQueryIndex:
    {
        json_t *object = eval_query(json, query->object, arena);
        if (!object)
            return NULL;

        if (object->kind != eJsonArray)
            return NULL;

        int idx = mpz_get_si(query->index);
        if (idx < 0)
            return NULL;

        if (idx >= typevec_len(&object->array))
            return NULL;

        return typevec_offset(&object->array, idx);
    }
    case eQueryMap:
    {
        json_t *object = eval_query(json, query->object, arena);
        if (!object)
            return NULL;

        if (object->kind != eJsonObject)
            return NULL;

        return json_map_get(object, arena_strndup(query->field.text, query->field.length, arena));
    }

    default:
        CT_NEVER("invalid query kind %d", query->kind);
    }
}

USE_DECL
json_t *json_query(json_t *json, const char *query, logger_t *logger, arena_t *arena)
{
    CTASSERT(json != NULL);
    CTASSERT(query != NULL);
    CTASSERT(arena != NULL);

    io_t *io = io_string("query", query, arena);

    query_scan_t ctx = {
        .reports = logger,
    };

    scan_t *scan = scan_io("json query", io, arena);
    scan_set_context(scan, &ctx);

    parse_result_t result = scan_buffer(scan, &kQueryCallbacks);

    if (result.result != eParseOk)
    {
        return NULL;
    }

    return eval_query(json, result.tree, arena);
}
