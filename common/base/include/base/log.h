#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdarg.h>
#include <stdbool.h>

BEGIN_API

typedef void (*verbose_t)(const char *fmt, va_list args);

typedef enum log_control_t
{
    eLogEnable,
    eLogDisable,
    eLogStatus,

    eLogCount
} log_control_t;

NODISCARD CONSTFN
verbose_t ctu_default_verbose(void);

void init_logs(IN_NOTNULL verbose_t callback);

log_control_t ctu_log_control(log_control_t control);

void ctu_log(FMT_STRING const char *fmt, ...) CT_PRINTF(1, 2);
void ctu_vlog(IN_STRING const char *fmt, va_list args);

END_API
