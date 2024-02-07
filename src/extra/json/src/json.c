#include "json/json.h"

#include "interop/compile.h"

#include "json_bison.h" // IWYU pragma: keep
#include "json_flex.h" // IWYU pragma: keep

CT_CALLBACKS(kCallbacks, json);

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
