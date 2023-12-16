#include "common.h"

#include "core/macros.h"
#include "scan/node.h"

const text_colour_t kDisabledColour = {
    .red = "",
    .green = "",
    .yellow = "",
    .blue = "",
    .magenta = "",
    .cyan = "",

    .reset = "",
};

const text_colour_t kDefaultColour = {
    .red = COLOUR_RED,
    .green = COLOUR_GREEN,
    .yellow = COLOUR_YELLOW,
    .blue = COLOUR_BLUE,
    .magenta = COLOUR_MAGENTA,
    .cyan = COLOUR_CYAN,

    .reset = COLOUR_RESET,
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

const char *get_severity_colour(text_colour_t colours, severity_t severity)
{
    switch (severity)
    {
    case eSeveritySorry: return colours.magenta;
    case eSeverityInternal: return colours.red;
    case eSeverityFatal: return colours.red;
    case eSeverityWarn: return colours.yellow;
    case eSeverityInfo: return colours.green;
    case eSeverityDebug: return colours.cyan;
    default: return "";
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
