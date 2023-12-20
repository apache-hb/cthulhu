#include "base/log.h"

#include <stdio.h>

static void default_verbose(const char *fmt, va_list args)
{
    (void)vprintf(fmt, args);
}

static verbose_t gVerboseFn = default_verbose;
static log_control_t gVerboseStatus = eLogDisable;

USE_DECL
verbose_t ctu_default_verbose(void)
{
    return default_verbose;
}

void init_logs(verbose_t callback)
{
    gVerboseFn = callback;
}

log_control_t ctu_log_control(log_control_t control)
{
    switch (control)
    {
    case eLogEnable: gVerboseStatus = eLogEnable; break;
    case eLogDisable: gVerboseStatus = eLogDisable; break;
    default: break;
    }

    return gVerboseStatus;
}

USE_DECL
void ctu_log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    ctu_vlog(fmt, args);

    va_end(args);
}

USE_DECL
void ctu_vlog(const char *fmt, va_list args)
{
    if (gVerboseStatus == eLogEnable)
    {
        gVerboseFn(fmt, args);
    }
}
