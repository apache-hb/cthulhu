#include "cthulhu/mediator/driver.h"
#include "scan/compile.h"

#include "cc/driver.h"

#include "core/macros.h"

#include "report/report.h"

#include "cc_bison.h"
#include "cc_flex.h"

// CTU_CALLBACKS(kCallbacks, cc);

static void cc_parse(driver_t *handle, scan_t *scan)
{
    CTU_UNUSED(handle);
    report(scan_reports(scan), eWarn, NULL, "C is unimplemented, ignoring file %s", scan_path(scan));
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

    .fnParse = cc_parse,

    .fnCreate = cc_create,
    .fnDestroy = cc_destroy,
};
