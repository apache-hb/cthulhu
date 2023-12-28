#pragma once

#include "core/compiler.h"
#include "core/analyze.h"

#include <stdarg.h>
#include <stdbool.h>

BEGIN_API

typedef void (*verbose_t)(const char *fmt, va_list args);

NODISCARD CONSTFN
verbose_t ctu_default_verbose(void);

void init_logs(IN_NOTNULL verbose_t callback);

void ctu_log_update(bool enable);
bool ctu_log_enabled(void);

CT_PRINTF(1, 2)
void ctu_log(FMT_STRING const char *fmt, ...);

void ctu_vlog(IN_STRING const char *fmt, va_list args);

END_API
