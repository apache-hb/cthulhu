// SPDX-License-Identifier: LGPL-3.0-only

#include "base/log.h"
#include "core/macros.h"

static void default_verbose(const char *fmt, va_list args)
{
    CT_UNUSED(fmt);
    CT_UNUSED(args);
}

verbose_t gVerboseCallback = default_verbose;
static bool gVerboseEnabled = false;

void ctu_log_update(bool enable)
{
    gVerboseEnabled = enable;
}

bool ctu_log_enabled(void)
{
    return gVerboseEnabled;
}

STA_DECL
void ctu_log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    ctu_vlog(fmt, args);

    va_end(args);
}

STA_DECL
void ctu_vlog(const char *fmt, va_list args)
{
    if (gVerboseEnabled)
    {
        gVerboseCallback(fmt, args);
    }
}
