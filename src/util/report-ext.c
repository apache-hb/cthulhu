#include "cthulhu/util/report-ext.h"

char *node_string(const node_t *node) {
    scan_t *scan = node->scan;
    where_t where = node->where;
    
    return format("%s source [%s:%ld:%ld]",
        scan->language, scan->path, 
        where.first_line + 1, where.first_column
    );
}

message_t *report_shadow(reports_t *reports,
                        const char *name,
                        const node_t *shadowed,
                        const node_t *shadowing)
{
    message_t *id = report(reports, ERROR, shadowing, "redefinition of `%s`", name);
    report_append(id, shadowed, "previous definition");
    return id;
}

message_t *report_unknown_character(reports_t *reports,
                                    const node_t *node,
                                    const char *str)
{
    where_t where = node->where;

    column_t width = where.last_column - where.first_column;
    char *normal = nstrnorm(str, MAX(width, 1));
    message_t *id = report(reports, ERROR, node, "unknown character `%s`", normal);
    
    return id;
}
