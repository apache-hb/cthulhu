// SPDX-License-Identifier: LGPL-3.0-only

#include "json/json.h"

#include "base/util.h"
#include "interop/compile.h"

#include "std/map.h"
#include "std/vector.h"

#include "json_bison.h" // IWYU pragma: keep
#include "json_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, json);

STA_DECL
json_t *json_map_get(const json_t *json, const char *key)
{
    CTASSERT(json != NULL);
    CTASSERT(key != NULL);

    // TODO: this isnt very good
    text_view_t view = text_view_from(key);
    return map_get(json->object, &view);
}

STA_DECL
json_t *json_array_get(const json_t *json, size_t index)
{
    CTASSERT(json != NULL);

    return typevec_offset(&json->array, index);
}

STA_DECL
json_t *json_scan(io_t *io, logger_t *logger, arena_t *arena)
{
    json_parse_t parse = json_parse(io, logger, arena);
    return parse.root;
}

STA_DECL
json_parse_t json_parse(io_t *io, logger_t *logger, arena_t *arena)
{
    CTASSERT(io != NULL);
    CTASSERT(logger != NULL);

    json_scan_t ctx = {
        .reports = logger,
    };

    scan_t *scan = scan_io("json", io, arena);
    scan_set_context(scan, &ctx);

    parse_result_t result = scan_buffer(scan, &kCallbacks);

    json_t *root = (result.result == eParseOk) ? result.tree : NULL;

    json_parse_t parse = {
        .scanner = scan,
        .root = root,
    };

    return parse;
}
