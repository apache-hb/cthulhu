#pragma once

#include "ctu/util/report.h"

typedef enum {
#define WARN(id, _) id,
#include "warnings.inc"
    WARN_MAX /// unknown warning
} warning_t;

warning_t get_warning_by_name(const char *name);
message_t *report_warn(reports_t *reports, warning_t err, const node_t *node, const char *fmt, ...);
