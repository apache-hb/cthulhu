#include "cthulhu/mediator/driver.h"
#include "interop/compile.h"

#include "cc/driver.h"

#include "core/macros.h"
#include "notify/notify.h"

#include "cc_bison.h" // IWYU pragma: keep
#include "cc_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, cc);

static void *cc_preparse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(handle);
    lifetime_t *lifetime = handle_get_lifetime(handle);
    logger_t *reports = lifetime_get_logger(lifetime);

    msg_notify(reports, &kEvent_DriverUnimplemented, node_builtin(), "C is unimplemented, ignoring file %s", scan_path(scan));

    return NULL;
}

static void cc_postparse(driver_t *handle, scan_t *scan, void *tree)
{
    CTU_UNUSED(tree);

    lifetime_t *lifetime = handle_get_lifetime(handle);
    logger_t *reports = lifetime_get_logger(lifetime);

    msg_notify(reports, &kEvent_DriverUnimplemented, node_builtin(), "C is unimplemented, ignoring file %s", scan_path(scan));
}

static const diagnostic_t * const kDiagnosticTable[] = {
#define NEW_EVENT(name, ...) &kEvent_##name,
#include "cc/events.inc"
};

static const char *const kLangNames[] = { "c", "h", NULL };

const language_t kCModule = {
    .id = "c",
    .name = "C",
    .version = {
        .license = "LGPLv3",
        .desc = "C11 language driver",
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
