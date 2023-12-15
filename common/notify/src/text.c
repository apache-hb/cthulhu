#include "notify/text.h"

#include "base/panic.h"

#include "core/macros.h"
#include "memory/memory.h"
#include "notify/notify.h"
#include "scan/node.h"

#include "io/io.h"

#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include <string.h>

typedef enum report_type_t
{
    eReportNoSource,
    eReport1Line,
    eReport2Line,
    eReport3Line,
    eReportManyLine,

    eReportTypeCount
} report_type_t;

static report_type_t get_report_type(const event_t *event)
{
    const node_t *node = event->node;
    if (!node_has_scanner(node)) return eReportNoSource;

    where_t where = node_get_location(node);
    size_t line_count = where.first_line - where.last_line + 1;

    switch (line_count)
    {
    case 1: return eReport1Line;
    case 2: return eReport2Line;
    case 3: return eReport3Line;
    default: return eReportManyLine;
    }
}

static const char *get_severity_name(severity_t severity)
{
    switch (severity)
    {
    case eSeveritySorry: return "unimplemented";
    case eSeverityInternal: return "internal";
    case eSeverityFatal: return "error";
    case eSeverityWarn: return "warning";
    case eSeverityInfo: return "info";
    case eSeverityDebug: return "debug";
    default: return "unknown";
    }
}

static const char *get_severity_colour(severity_t severity)
{
    switch (severity)
    {
    case eSeveritySorry: return COLOUR_PURPLE;
    case eSeverityInternal: return COLOUR_RED;
    case eSeverityFatal: return COLOUR_RED;
    case eSeverityWarn: return COLOUR_YELLOW;
    case eSeverityInfo: return COLOUR_CYAN;
    case eSeverityDebug: return COLOUR_GREEN;
    default: return "";
    }
}

static const char *colour(text_config_t config, const char *colour)
{
    return config.colour ? colour : "";
}

// static const char *coloured(text_config_t config, const char *text, const char *colour)
// {
//     if (!config.colour) return text;

//     return format("%s%s%s", colour, text, COLOUR_RESET);
// }

// print `severity [id]: message`
static void print_header(text_config_t config, const event_t *event)
{
    const diagnostic_t *diagnostic = event->diagnostic;
    const char *sev = get_severity_name(diagnostic->severity);
    const char *col = get_severity_colour(diagnostic->severity);

    io_printf(config.io, "%s%s[%s]%s: %s\n", colour(config, col), sev, diagnostic->id, colour(config, COLOUR_RESET), event->message);
}

typedef struct print_result_t
{
    size_t column_align;
} print_result_t;

static size_t number_width(size_t number)
{
    size_t width = 0;
    while (number > 0)
    {
        number /= 10;
        width++;
    }

    return width;
}

static const char *get_scan_name(const node_t *node)
{
    if (node_is_builtin(node)) return "internal";

    const scan_t *scan = node_get_scan(node);
    return scan_path(scan);
}

static void print_location(text_config_t config, where_t where, const char *lang, size_t col_align)
{
    char *padding = str_repeat(" ", col_align);
    io_printf(config.io, "%s=> [%s:%" PRI_LINE ":%" PRI_LINE "]\n", padding, lang, where.first_line, where.first_column);
}

static const char *extract_line(const scan_t *scan, size_t line)
{
    const char *text = scan_text(scan);
    size_t len = scan_size(scan);

    size_t cur_line = 0;
    size_t start = 1;
    for (size_t i = 0; i < len; i++)
    {
        if (text[i] == '\n')
        {
            cur_line++;

            if (cur_line == line)
            {
                break;
            }
        }

        start++;
    }

    // once we've found the start now walk to the next newline

    size_t end = start + 1;
    for (size_t i = start; i < len; i++)
    {
        if (text[i] == '\n')
        {
            break;
        }

        end++;
    }

    if (end == start) return "<oops>";

    return ctu_strndup(text + start, end - start - 1);
}

static size_t get_line(text_config_t config, size_t line)
{
    if (config.zero_line) return line;

    return line - 1;
}

static const char *build_underline(text_config_t config, const char *text, const event_t *event)
{
    if (event->underline == NULL) return "";

    // extract the proper spacing characters from the source text for the underline
    where_t where = node_get_location(event->node);

    size_t col = where.first_column;

    typevec_t *spacing = typevec_new(sizeof(char), col);
    for (size_t i = 0; i < col; i++)
    {
        char c = text[i];
        if (c == '\t')
        {
            typevec_push(spacing, "\t");
        }
        else
        {
            typevec_push(spacing, " ");
        }
    }
    typevec_push(spacing, "\0");

    size_t span = where.last_column - col;
    if (span == 0) span = 1;
    char *dash = str_repeat("-", span - 1);

    return format("%s%s^%s%s %s", typevec_data(spacing), colour(config, COLOUR_BLUE), dash, colour(config, COLOUR_RESET), event->underline);
}

static print_result_t print_single_line(text_config_t config, const event_t *event)
{
    const scan_t *scan = node_get_scan(event->node);
    const char *name = get_scan_name(event->node);
    where_t where = node_get_location(event->node);
    size_t line = get_line(config, where.first_line);
    size_t col_align = number_width(line);

    // +2 as theres a space either side of the column number
    print_location(config, where, name, col_align + 2);

    print_result_t result = {
        .column_align = col_align + 2
    };

    const char *text = extract_line(scan, where.first_line);

    const char *underline = build_underline(config, text, event);

    const char *padding = str_repeat(" ", col_align);

    io_printf(config.io, " %s |\n", padding);
    io_printf(config.io, " %" PRI_LINE " |%s\n", line, text);
    io_printf(config.io, " %s |%s\n", padding, underline);

    return result;
}

static void print_notes(text_config_t config, vector_t *notes, size_t col_align)
{
    if (notes == NULL) return;
    if (vector_len(notes) == 0) return;

    if (col_align < 4) col_align = 0;
    else col_align -= 4;

    char *padding = str_repeat(" ", col_align);
    char *extra_padding = str_repeat(" ", col_align + 5); // +5 for note:

    // print the first note with the `note:` heading
    const char *first = vector_get(notes, 0);

    io_printf(config.io, "%s%snote:%s %s\n", padding, colour(config, COLOUR_YELLOW), colour(config, COLOUR_RESET), first);

    // print the rest of the notes without the heading
    size_t count = vector_len(notes);
    for (size_t i = 1; i < count; i++)
    {
        const char *note = vector_get(notes, i);
        io_printf(config.io, "%s %s\n", extra_padding, note);
    }
}

void text_report(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);

    report_type_t type = get_report_type(event);

    print_header(config, event);

    print_result_t result = { 0 };

    switch (type)
    {
    case eReport1Line:
        result = print_single_line(config, event);

    default:
        break;
    }

    print_notes(config, event->notes, result.column_align);
}
