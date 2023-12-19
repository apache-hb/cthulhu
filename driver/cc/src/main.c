#include "cthulhu/mediator/driver.h"
#include "interop/compile.h"

#include "cc/driver.h"

#include "core/macros.h"

#include "report/report.h"

#include "cc_bison.h"
#include "cc_flex.h"

CTU_CALLBACKS(kCallbacks, cc);

static void cc_preparse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(handle);
    lifetime_t *lifetime = handle_get_lifetime(handle);
    reports_t *reports = lifetime_get_reports(lifetime);

    report(reports, eWarn, node_builtin(), "C is unimplemented, ignoring file %s", scan_path(scan));
}

static void cc_postparse(driver_t *handle, scan_t *scan, void *tree)
{
    CTU_UNUSED(tree);

    lifetime_t *lifetime = handle_get_lifetime(handle);
    reports_t *reports = lifetime_get_reports(lifetime);

    report(reports, eWarn, node_builtin(), "C is unimplemented, ignoring file %s", scan_path(scan));
}

static const char *kLangNames[] = { "c", "h", NULL };

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

    .fnCreate = cc_create,
    .fnDestroy = cc_destroy,

    .fn_prepass = cc_preparse,
    .fn_postpass = cc_postparse,
    .callbacks = &kCallbacks,
};
