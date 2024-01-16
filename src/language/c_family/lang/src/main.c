#include "base/log.h"
#include "cthulhu/runtime/driver.h"
#include "interop/compile.h"

#include "c/driver.h"

#include "core/macros.h"
#include "arena/arena.h"
#include "notify/notify.h"

#include "driver/driver.h"

#include "cc_bison.h" // IWYU pragma: keep
#include "cc_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, cc);

static void *cc_preparse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(scan);

    lifetime_t *lifetime = handle_get_lifetime(handle);
    logger_t *reports = lifetime_get_logger(lifetime);
    arena_t *arena = lifetime_get_arena(lifetime);

    cc_scan_t it = {
        .reports = reports,
    };

    cc_scan_t *ptr = arena_memdup(&it, sizeof(cc_scan_t), arena);
    return ptr;
}

static void cc_postparse(driver_t *handle, scan_t *scan, void *tree)
{
    CTU_UNUSED(handle);
    CTU_UNUSED(scan);
    CTU_UNUSED(tree);

    ctu_log("cc_postparse");
}

static const diagnostic_t * const kDiagnosticTable[] = {
#define NEW_EVENT(name, ...) &kEvent_##name,
#include "c/events.def"
};

static const char *const kLangNames[] = { "c", "h", NULL };

CT_DRIVER_API const language_t kCModule = {
    .id = "c",
    .name = "C",
    .version = {
        .license = "LGPLv3",
        .desc = "C language driver",
        .author = "Elliot Haisley",
        .version = NEW_VERSION(0, 0, 1),
    },

    .exts = kLangNames,

    .diagnostics = {
        .diagnostics = kDiagnosticTable,
        .count = sizeof(kDiagnosticTable) / sizeof(diagnostic_t*),
    },

    .fn_create = cc_create,
    .fn_destroy = cc_destroy,

    .fn_preparse = cc_preparse,
    .fn_postparse = cc_postparse,
    .parse_callbacks = &kCallbacks,
};

CTU_DRIVER_ENTRY(kCModule)
