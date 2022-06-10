#include "report/report-ext.h"

#include "report/report.h"

#include "base/macros.h"
#include "std/str.h"

message_t *report_shadow(reports_t *reports, const char *name, node_t prevDefinition, node_t newDefinition)
{
    message_t *id = report(reports, ERROR, newDefinition, "redefinition of `%s`", name);
    report_append(id, prevDefinition, "previous definition");
    return id;
}

message_t *report_unknown_character(reports_t *reports, node_t node, const char *str)
{
    where_t where = get_node_location(node);

    column_t width = where.lastColumn - where.firstColumn;
    char *normal = str_normalizen(str, MAX(width, 1));
    message_t *id = report(reports, ERROR, node, "unknown character `%s`", normal);

    return id;
}
