#include "common.h"

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
#include <stdlib.h>
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

/// @brief order segments by their location in the source file
static int segment_order(const void *lhs, const void *rhs)
{
    const segment_t *a = lhs;
    const segment_t *b = rhs;

    if (a->node == NULL) return -1;
    if (b->node == NULL) return 1;

    where_t a_where = node_get_location(a->node);
    where_t b_where = node_get_location(b->node);

    if (a_where.first_line < b_where.first_line) return -1;
    if (a_where.first_line > b_where.first_line) return 1;

    if (a_where.first_column < b_where.first_column) return -1;
    if (a_where.first_column > b_where.first_column) return 1;

    return 0;
}

/// @brief sort segments based on the order we want to print them
static typevec_t *order_segments(typevec_t *segments, const scan_t *scan)
{
    // only returns segments that have the same source file as scan
    // segments are ordered by their location in the source file

    size_t len = typevec_len(segments);

    typevec_t *result = typevec_new(sizeof(segment_t), len);

    for (size_t i = 0; i < len; i++)
    {
        segment_t *segment = typevec_offset(segments, i);

        if (segment->node == NULL) continue;
        if (!node_has_scanner(segment->node) || node_get_scan(segment->node) != scan) continue;

        typevec_push(result, segment);
    }

    // sort the segments by their location in the source file
    segment_t *data = typevec_data(result);
    qsort(data, typevec_len(result), sizeof(segment_t), segment_order);

    return result;
}

// print `severity [id]: message`
static void print_header(text_config_t config, const event_t *event)
{
    const diagnostic_t *diagnostic = event->diagnostic;
    const char *sev = get_severity_name(diagnostic->severity);
    colour_t col = get_severity_colour(diagnostic->severity);
    const char *it = config.colours.colours[col];

    io_printf(config.io, "%s%s[%s]%s: %s\n", it, sev, diagnostic->id, config.colours.reset, event->message);
}

typedef struct print_result_t
{
    size_t column_align;
} print_result_t;

static size_t number_width(line_t number)
{
    size_t width = 0;
    while (number > 0)
    {
        number /= 10;
        width++;
    }

    return width;
}

static void print_location(text_config_t config, where_t where, const char *lang, size_t col_align)
{
    char *padding = str_repeat(" ", col_align);
    io_printf(config.io, " %s => [%s:%" PRI_LINE "]\n", padding, lang, where.first_line);
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

static size_t get_line(file_config_t config, size_t line)
{
    if (config.zeroth_line) return line;

    return line + 1;
}

static typevec_t *collect_space(const char *text, size_t col)
{
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

    return spacing;
}

static char *align_number_right(line_t line, size_t col_align)
{
    char *line_str = format("%" PRI_LINE, line);
    size_t len = strlen(line_str);

    if (len >= col_align) return line_str;

    size_t diff = col_align - len;
    char *padding = str_repeat(" ", diff);

    char *result = format("%s%s", padding, line_str);

    ctu_free(line_str);
    ctu_free(padding);

    return result;
}

static void print_segment(text_config_t config, const node_t *node, const char *message, size_t col_align)
{
    where_t where = node_get_location(node);
    const scan_t *scan = node_get_scan(node);

    const char *text = extract_line(scan, where.first_line);

    typevec_t *spacing = collect_space(text, where.first_column);

    char *line = align_number_right(get_line(config.config, where.first_line), col_align);
    char *space = str_repeat(" ", col_align);

    char *spacestr = typevec_data(spacing);

    io_printf(config.io, " %s |%s\n", space, text);
    io_printf(config.io, " %s |%s%s\n", line, spacestr, message);
    io_printf(config.io, " %s |\n", space);
}

static print_result_t print_single_line(text_config_t config, const event_t *event)
{
    const scan_t *scan = node_get_scan(event->node);
    const char *name = get_scan_name(event->node);
    where_t where = node_get_location(event->node);
    line_t primary_line = get_line(config.config, where.first_line);

    typevec_t *parts = order_segments(event->segments, scan);
    size_t len = typevec_len(parts);

    // get the largest number width
    size_t col_align = number_width(primary_line);
    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(parts, i);
        where_t segment_where = node_get_location(segment->node);
        line_t line = get_line(config.config, segment_where.first_line);
        size_t width = number_width(line);
        col_align = MAX(col_align, width);
    }

    print_location(config, where, name, col_align);

    // const char *text = extract_line(scan, where.first_line);
    // const char *padding = str_repeat(" ", col_align);

    // // if the first character is not whitespace then add a space to the start of the line
    // const char *extra = "";
    // if (!char_is_any_of(text[0], STR_WHITESPACE))
    //     extra = " ";

    print_segment(config, event->node, event->message, col_align);

    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(parts, i);
        print_segment(config, segment->node, segment->message, col_align);
    }

    // io_printf(config.io, " %s |\n", padding);
    // io_printf(config.io, " %" PRI_LINE " |%s%s\n", primary_line, extra, text);
    // io_printf(config.io, " %s |\n", padding);

    print_result_t result = {
        .column_align = col_align + 2
    };

    return result;
}

static print_result_t print_many_line(text_config_t config, const event_t *event)
{
    const scan_t *scan = node_get_scan(event->node);
    const char *name = get_scan_name(event->node);
    where_t where = node_get_location(event->node);
    size_t first_line = get_line(config.config, where.first_line);
    size_t last_line = get_line(config.config, where.last_line);
    size_t first_col_align = number_width(first_line);
    size_t last_col_align = number_width(last_line);

    // +2 as theres a space either side of the column number
    size_t max_align = MAX(first_col_align, last_col_align);
    print_location(config, where, name, max_align);

    const char *first_padding = str_repeat(" ", last_col_align - first_col_align);

    const char *first_text = extract_line(scan, where.first_line);
    const char *last_text = extract_line(scan, where.last_line);
    const char *padding = str_repeat(" ", max_align);

    const char *extra = "";
    if (!char_is_any_of(first_text[0], STR_WHITESPACE))
        extra = " ";

    io_printf(config.io, " %s |\n", padding);
    io_printf(config.io, " %s%" PRI_LINE " >%s%s\n", first_padding, first_line, extra, first_text);
    io_printf(config.io, " %s ...\n", padding);
    io_printf(config.io, " %" PRI_LINE " >%s%s\n", last_line, extra, last_text);
    io_printf(config.io, " %s |%s\n", padding, extra);

    print_result_t result = {
        .column_align = max_align + 2
    };

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

    const char *heading = fmt_coloured(config.colours, eColourYellow, "note:");

    io_printf(config.io, "%s%s %s\n", padding, heading, first);

    // print the rest of the notes without the heading
    size_t count = vector_len(notes);
    for (size_t i = 1; i < count; i++)
    {
        const char *note = vector_get(notes, i);
        io_printf(config.io, "%s %s\n", extra_padding, note);
    }
}

void text_report_rich(text_config_t config, const event_t *event)
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
        break;

    case eReportManyLine:
        result = print_many_line(config, event);

    default:
        break;
    }

    print_notes(config, event->notes, result.column_align);
}
