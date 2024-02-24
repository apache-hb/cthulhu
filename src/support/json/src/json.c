// SPDX-License-Identifier: LGPL-3.0-only

#include "json/json.h"

#include "interop/compile.h"

#include "std/map.h"
#include "std/vector.h"

#include "json_bison.h" // IWYU pragma: keep
#include "json_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, json);

USE_DECL
json_t *json_map_get(const json_t *json, const char *key)
{
    CTASSERT(json != NULL);
    CTASSERT(key != NULL);

    return map_get(json->object, key);
}

USE_DECL
json_t *json_array_get(const json_t *json, size_t index)
{
    CTASSERT(json != NULL);

    return vector_get(json->array, index);
}

json_t *json_scan(io_t *io, logger_t *logger, arena_t *arena)
{
    CTASSERT(io != NULL);
    CTASSERT(logger != NULL);

    json_scan_t ctx = {
        .reports = logger,
    };

    scan_t *scan = scan_io("json", io, arena);
    scan_set_context(scan, &ctx);

    parse_result_t result = scan_buffer(scan, &kCallbacks);

    if (result.result == eParseOk)
        return result.tree;

    return NULL;
}
