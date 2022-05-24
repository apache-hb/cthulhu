#include "cthulhu/report/report-ext.h"

#include "cthulhu/util/str.h"
#include "cthulhu/util/macros.h"

message_t *report_shadow(reports_t *reports, const char *name, node_t shadowed, node_t shadowing)
{
    message_t *id = report(reports, ERROR, shadowing, "redefinition of `%s`", name);
    report_append(id, shadowed, "previous definition");
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
