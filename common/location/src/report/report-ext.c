#include "report/report-ext.h"

#include "report/report.h"

#include "core/macros.h"
#include "std/str.h"

message_t *report_shadow(reports_t *reports, const char *name, const node_t *previous, const node_t *redefine)
{
    message_t *id = report(reports, eFatal, redefine, "redefinition of `%s`", name);
    report_append(id, previous, "previous definition");
    return id;
}

message_t *report_unknown_character(reports_t *reports, const node_t *node, const char *str)
{
    where_t where = get_node_location(node);

    column_t width = where.lastColumn - where.firstColumn;
    char *normal = str_normalizen(str, MAX(width, 1));
    message_t *id = report(reports, eFatal, node, "unknown character `%s`", normal);

    return id;
}

message_t *report_os(reports_t *reports, const char *msg, os_error_t err)
{
    message_t *id = report(reports, eFatal, node_invalid(), "%s", msg);
    report_note(id, "%s", os_error_string(err));
    return id;
}
