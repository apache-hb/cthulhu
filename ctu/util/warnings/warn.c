#include "report.h"

#include "ctu/util/str.h"

warning_t get_warning_by_name(const char *name) {
#define WARN(id, str) if (streq(name, str)) { return id; }
#include "warnings.inc"
    return WARN_MAX;
}

message_t *report_warn(reports_t *reports, warning_t err, const node_t *node, const char *fmt, ...) {

}
