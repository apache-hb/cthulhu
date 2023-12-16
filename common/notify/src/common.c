#include "common.h"

#include "notify/format.h"

#include "base/panic.h"
#include "core/macros.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include <string.h>

const text_colour_t kDisabledColour = {
    .colours = {
        [eColourRed] = "",
        [eColourGreen] = "",
        [eColourYellow] = "",
        [eColourBlue] = "",
        [eColourMagenta] = "",
        [eColourCyan] = "",
        [eColourWhite] = "",
        [eColourDefault] = "",
    },

    .reset = "",
};

const text_colour_t kDefaultColour = {
    .colours = {
        [eColourRed] = ANSI_RED,
        [eColourGreen] = ANSI_GREEN,
        [eColourYellow] = ANSI_YELLOW,
        [eColourBlue] = ANSI_BLUE,
        [eColourMagenta] = ANSI_MAGENTA,
        [eColourCyan] = ANSI_CYAN,
        [eColourWhite] = ANSI_WHITE,
        [eColourDefault] = ANSI_DEFAULT,
    },

    .reset = ANSI_RESET,
};


const char *get_severity_name(severity_t severity)
{
    switch (severity)
    {
    case eSeveritySorry: return "sorry";
    case eSeverityInternal: return "internal";
    case eSeverityFatal: return "error";
    case eSeverityWarn: return "warning";
    case eSeverityInfo: return "info";
    case eSeverityDebug: return "debug";
    default: return "unknown";
    }
}

colour_t get_severity_colour(severity_t severity)
{
    switch (severity)
    {
    case eSeveritySorry: return eColourMagenta;
    case eSeverityInternal: return eColourCyan;
    case eSeverityFatal: return eColourRed;
    case eSeverityWarn: return eColourYellow;
    case eSeverityInfo: return eColourGreen;
    case eSeverityDebug: return eColourGreen;
    default: return eColourDefault;
    }
}

const char *get_scan_name(const node_t *node)
{
    if (node_is_builtin(node)) return "builtin";
    if (node_is_invalid(node)) return "invalid";

    if (!node_has_scanner(node)) return "unknown";

    const scan_t *scan = node_get_scan(node);
    return scan_path(scan);
}

// assumes all segments are in the same file
static int segment_cmp(const void *lhs, const void *rhs)
{
    const segment_t *seg_lhs = lhs;
    const segment_t *seg_rhs = rhs;

    const scan_t *scan_lhs = node_get_scan(seg_lhs->node);
    const scan_t *scan_rhs = node_get_scan(seg_rhs->node);

    CTASSERTF(scan_lhs == scan_rhs, "segments must be in the same scan (%s and %s)", scan_path(scan_lhs), scan_path(scan_rhs));

    where_t where_lhs = node_get_location(seg_lhs->node);
    where_t where_rhs = node_get_location(seg_rhs->node);

    if (where_lhs.first_line < where_rhs.first_line) return -1;
    if (where_lhs.first_line > where_rhs.first_line) return 1;

    if (where_lhs.first_column < where_rhs.first_column) return -1;
    if (where_lhs.first_column > where_rhs.first_column) return 1;

    return 0;
}

void segments_sort(typevec_t *segments)
{
    typevec_sort(segments, segment_cmp);
}

typevec_t *all_segments_in_scan(const typevec_t *segments, const node_t *node)
{
    CTASSERT(segments != NULL);
    CTASSERT(node != NULL);

    const scan_t *scan = node_get_scan(node);

    size_t len = typevec_len(segments);
    typevec_t *result = typevec_new(sizeof(segment_t), len);

    for (size_t i = 0; i < len; i++)
    {
        segment_t *segment = typevec_offset(segments, i);
        CTASSERT(segment != NULL);

        const scan_t *other = node_get_scan(segment->node);
        if (other != scan) continue;

        typevec_push(result, segment);
    }

    segments_sort(result);

    return result;
}

char *fmt_node(file_config_t config, const node_t *node)
{
    where_t where = node_get_location(node);
    const char *path = get_scan_name(node);

    if (node_has_scanner(node))
    {
        line_t first_line = get_line_number(config, node);

        return format("%s:%" PRI_LINE ":%" PRI_COLUMN "", path, first_line,
                      where.first_column);
    }
    else
    {
        return format("<%s>", path);
    }
}

char *fmt_coloured(text_colour_t colours, colour_t idx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = vformat(fmt, args);
    va_end(args);

    return format("%s%s%s", colours.colours[idx], msg, colours.reset);
}

line_t get_line_number(file_config_t config, const node_t *node)
{
    where_t where = node_get_location(node);
    if (config.zeroth_line) return where.first_line;

    return where.first_line + 1;
}
