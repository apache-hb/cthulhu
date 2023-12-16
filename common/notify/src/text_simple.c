#include "common.h"

#include "base/panic.h"
#include "scan/node.h"
#include "io/io.h"

USE_DECL
void text_report_simple(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);
    const diagnostic_t *diagnostic = event->diagnostic;

    const char *sev = get_severity_name(diagnostic->severity);
    const char *col = get_severity_colour(config.colours, diagnostic->severity);

    const node_t *node = event->node;
    bool use_loc = node_has_scanner(node);
    where_t where = node_get_location(node);

    const char *path = get_scan_name(node);

    if (use_loc)
    {
        io_printf(config.io, "%s:%" PRI_LINE ":%" PRI_COLUMN ": %s%s:%s %s [%s]\n", path, where.first_line, where.first_column, col, sev, config.colours.reset, event->message, diagnostic->id);
    }
    else
    {
        io_printf(config.io, "%s: %s%s:%s %s [%s]\n", path, col, sev, config.colours.reset, event->message, diagnostic->id);
    }
}
