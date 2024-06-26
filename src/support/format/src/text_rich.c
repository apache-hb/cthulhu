// SPDX-License-Identifier: LGPL-3.0-only

#include "common_extra.h"

#include "format/notify.h"

#include "memory/memory.h"

#include "scan/node.h"

#include "io/io.h"

#include "std/set.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include "arena/arena.h"

#include "base/util.h"
#include "base/panic.h"

#include "core/macros.h"

#include <ctype.h>

#define COLOUR_PATH eColourBlue
#define COLOUR_UNDERLINE eColourMagenta
#define COLOUR_SEGMENT eColourGreen
#define COLOUR_NOTE_STAR eColourYellow
#define COLOUR_MESSAGE eColourRed

typedef struct rich_t
{
    arena_t *arena;

    text_config_t config;
    const event_t *event;

    severity_t severity;

    // the largest line number in the report
    size_t largest_line;

    cache_map_t *file_cache;

    size_t max_columns;

    format_context_t fmt;
} rich_t;

static void print_report_header(rich_t *rich, const char *message)
{
    CTASSERT(rich != NULL);
    CTASSERT(message != NULL);

    const diagnostic_t *diag = rich->event->diagnostic;
    text_config_t config = rich->config;

    const char *sev = get_severity_name(rich->severity);
    colour_t colour = get_severity_colour(rich->severity);

    char *coloured = colour_format(rich->fmt, colour, "%s [%s]:", sev, diag->id);

    io_printf(config.io, "%s %s\n", coloured, message);
}

static where_t get_first_line(const typevec_t *segments)
{
    CTASSERT(segments != NULL);

    size_t len = typevec_len(segments);
    size_t first_line = SIZE_MAX;

    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(segments, i);
        CTASSERT(segment != NULL);

        if (!node_has_line(&segment->node))
            continue;

        where_t where = node_get_location(&segment->node);
        first_line = CT_MIN(first_line, where.first_line);
        break;
    }

    where_t where = {
        .first_line = first_line,
        .first_column = 0,
        .last_line = first_line,
        .last_column = 0,
    };

    return where;
}

static void print_scan_header(rich_t *rich, size_t largest, size_t line, const scan_t *scan)
{
    CTASSERT(rich != NULL);
    CTASSERT(scan != NULL);

    text_config_t config = rich->config;

    const char *name = scan_path(scan);
    const char *lang = scan_language(scan);
    char *padding = str_repeat(" ", get_num_width(largest), rich->arena);

    if (scan_is_builtin(scan))
    {
        char *coloured = colour_format(rich->fmt, COLOUR_PATH, "<%s>", name);

        io_printf(config.io, " %s => %s\n", padding, coloured);
    }
    else
    {
        size_t display_line = get_offset_line(config.config.zeroth_line, line);

        io_printf(config.io, " %s => %s [%s:%zu]\n", padding, lang, name, display_line);
    }
}

static void print_file_header(rich_t *rich, const node_t *node)
{
    CTASSERT(rich != NULL);
    CTASSERT(node != NULL);

    const scan_t *scan = node_get_scan(node);
    text_config_t config = rich->config;
    int width = get_num_width(rich->largest_line);
    size_t line = get_line_number(config.config, node);

    if (node_has_line(node))
    {
        const char *name = scan_path(scan);
        const char *lang = scan_language(scan);

        char *padding = str_repeat(" ", width, rich->arena);

        io_printf(config.io, " %s => %s [%s:%zu]\n", padding, lang, name, line);
    }
    else
    {
        print_scan_header(rich, width, 0, scan);
    }
}

static char *fmt_underline(text_cache_t *cache, const node_t *node, size_t limit, arena_t *arena)
{
    CTASSERT(cache != NULL);

    where_t where = node_get_location(node);

    text_view_t view = cache_get_line(cache, where.first_line);
    size_t width = CT_MIN(view.length, limit);

    typevec_t *padding = typevec_new(sizeof(char), width, arena);
    for (size_t i = 0; i < width; i++)
    {
        if (i >= where.first_column) break;

        char c = view.text[i];
        typevec_push(padding, c == '\t' ? "\t" : " ");
    }
    typevec_push(padding, "\0");

    size_t underline_width = width;
    if (where.first_line == where.last_line)
    {
        underline_width = CT_MIN(where.last_column - where.first_column, 1);
    }

    const char *lines = (underline_width > 1) ? str_repeat("~", underline_width - 1, arena) : "";
    char *underline = str_format(arena, "%s^%s", (char*)typevec_data(padding), lines);

    return underline;
}

static void print_file_segment(rich_t *rich, const node_t *node, const char *message)
{
    CTASSERT(rich != NULL);
    CTASSERT(node != NULL);
    CTASSERT(message != NULL);

    text_config_t config = rich->config;

    const scan_t *scan = node_get_scan(node);
    where_t where = node_get_location(node);
    size_t data_line = where.first_line;

    size_t display_line = get_line_number(config.config, node);
    int width = get_num_width(CT_MAX(display_line, rich->largest_line));
    char *padding = str_repeat(" ", width, rich->arena);
    char *line = fmt_left_align(rich->arena, width, "%zu", display_line);

    text_cache_t *file = cache_emplace_scan(rich->file_cache, scan);
    text_t source = cache_escape_line(file, data_line, config.colours, rich->max_columns);

    // get the first line of the message
    vector_t *lines = str_split(message, "\n", rich->arena);
    char *first = vector_get(lines, 0);

    if (vector_len(lines) > 1)
    {
        char *one = colour_text(rich->fmt, COLOUR_SEGMENT, "(1)");
        first = str_format(rich->arena, "%s %s", one, first);
    }

    char *underline = fmt_underline(file, node, rich->max_columns, rich->arena);
    char *coloured_underline = colour_text(rich->fmt, COLOUR_UNDERLINE, underline);

    const char *pretext = !isspace(source.text[0]) ? " " : "";

    io_printf(config.io, " %s |\n", padding);
    io_printf(config.io, " %s |%s%.*s\n", line, pretext, (int)source.length, source.text);
    io_printf(config.io, " %s |%s%s %s.\n", padding, pretext, coloured_underline, first);

    size_t extra_padding = ctu_strlen(underline);
    char *extra = str_repeat(" ", extra_padding, rich->arena);

    size_t len = vector_len(lines);
    int align = get_num_width(len);

    for (size_t i = 1; i < len; i++)
    {
        const char *it = vector_get(lines, i);
        char *aligned = fmt_left_align(rich->arena, align, "(%zu)", i + 1);
        char *coloured = colour_text(rich->fmt, COLOUR_SEGMENT, aligned);
        io_printf(config.io, " %s |%s%s %s %s.\n", padding, pretext, extra, coloured, it);
    }
}

static void print_segment_message(rich_t *rich, const char *message)
{
    CTASSERT(rich != NULL);
    CTASSERT(message != NULL);

    text_config_t config = rich->config;

    vector_t *lines = str_split(message, "\n", rich->arena);
    size_t len = vector_len(lines);

    for (size_t i = 0; i < len; i++)
    {
        char *it = vector_get(lines, i);
        char *coloured = colour_format(rich->fmt, COLOUR_SEGMENT, "(%zu)", i + 1);
        io_printf(config.io, "  %s %s\n", coloured, it);
    }
}

static void print_file_segments(rich_t *rich, const typevec_t *segments)
{
    CTASSERT(rich != NULL);
    CTASSERT(segments != NULL);

    size_t len = typevec_len(segments);
    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(segments, i);
        CTASSERT(segment != NULL);

        if (node_has_line(&segment->node))
        {
            print_file_segment(rich, &segment->node, segment->message);
        }
        else
        {
            // just print the message
            print_segment_message(rich, segment->message);
        }
    }
}

static bool nodes_overlap(const node_t *lhs, const node_t *rhs)
{
    CTASSERT(lhs != NULL);
    CTASSERT(rhs != NULL);

    where_t lhs_where = node_get_location(lhs);
    where_t rhs_where = node_get_location(rhs);

    // if the elements overlap at all then they are considered to be the same

    if (lhs_where.first_line <= rhs_where.first_line &&
        lhs_where.last_line >= rhs_where.first_line)
    {
        return true;
    }

    if (lhs_where.first_line <= rhs_where.last_line &&
        lhs_where.last_line >= rhs_where.last_line)
    {
        return true;
    }

    return false;
}

static typevec_t *collect_segments(rich_t *rich, const typevec_t *all, const scan_t *scan)
{
    CTASSERT(rich != NULL);
    CTASSERT(scan != NULL);

    if (all == NULL)
        return typevec_new(sizeof(segment_t), 0, rich->arena);

    size_t count = typevec_len(all);
    typevec_t *primary = typevec_new(sizeof(segment_t), count, rich->arena);
    for (size_t i = 0; i < count; i++)
    {
        const segment_t *segment = typevec_offset(all, i);
        CTASSERT(segment != NULL);

        const node_t *other = &segment->node;
        if (node_get_scan(other) != scan)
            continue;

        if (node_has_line(other))
        {
            where_t where = node_get_location(other);
            rich->largest_line = CT_MAX(rich->largest_line, where.first_line);
        }

        typevec_push(primary, segment);
    }

    return primary;
}

typedef struct join_result_t
{
    bool joined_nodes;
    bool used_primary;
} join_result_t;

static join_result_t join_node_messages(rich_t *rich, const segment_t *segment, segment_t *other, const char *primary)
{
    CTASSERT(rich != NULL);
    CTASSERT(segment != NULL);
    CTASSERT(other != NULL);

    if (!nodes_overlap(&segment->node, &other->node))
    {
        join_result_t result = {
            .joined_nodes = false,
            .used_primary = false,
        };

        return result;
    }

    join_result_t result = {
        .joined_nodes = true,
        .used_primary = false,
    };

    if (segment->message == primary)
    {
        result.used_primary = true;

        char *coloured = colour_text(rich->fmt, COLOUR_MESSAGE, segment->message);
        other->message = str_format(rich->arena, "%s\n%s", coloured, other->message);
    }
    else if (other->message == primary)
    {
        result.used_primary = true;

        char *coloured = colour_text(rich->fmt, COLOUR_MESSAGE, other->message);
        other->message = str_format(rich->arena, "%s\n%s", segment->message, coloured);
    }
    else
    {
        other->message = str_format(rich->arena, "%s\n%s", segment->message, other->message);
    }

    return result;
}

static typevec_t *merge_segments(rich_t *rich, const typevec_t *segments, const char *primary)
{
    CTASSERT(rich != NULL);
    CTASSERT(segments != NULL);

    // now merge segments that share the same span
    size_t len = typevec_len(segments);
    typevec_t *result = typevec_new(sizeof(segment_t), len, rich->arena);
    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(segments, i);
        CTASSERT(segment != NULL);

        bool found = false;
        for (size_t j = 0; j < typevec_len(result); j++)
        {
            segment_t *other = typevec_offset(result, j);
            CTASSERT(other != NULL);

            join_result_t join = join_node_messages(rich, segment, other, primary);

            if (join.joined_nodes)
            {
                if (join.used_primary)
                {
                    primary = NULL;
                }

                found = true;
                break;
            }
        }

        if (!found)
        {
            typevec_push(result, segment);
        }
    }

    return result;
}

static typevec_t *collect_primary_segments(rich_t *rich, const typevec_t *all, const event_t *event)
{
    CTASSERT(rich != NULL);
    CTASSERT(event != NULL);

    const scan_t *scan = node_get_scan(&event->node);
    typevec_t *primary = collect_segments(rich, all, scan);

    segment_t event_segment = {
        .node = event->node,
        .message = event->message,
    };

    typevec_push(primary, &event_segment);

    // now merge segments that share the same span
    typevec_t *result = merge_segments(rich, primary, event->message);

    return result;
}

static size_t longest_segment_line(const typevec_t *segments)
{
    CTASSERT(segments != NULL);

    size_t len = typevec_len(segments);
    size_t longest = 0;

    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(segments, i);
        CTASSERT(segment != NULL);

        if (!node_has_line(&segment->node))
            continue;

        where_t where = node_get_location(&segment->node);
        longest = CT_MAX(longest, where.first_line);
    }

    return longest;
}

static void print_extra_files(rich_t *rich)
{
    CTASSERT(rich != NULL);
    const event_t *event = rich->event;

    if (event->segments == NULL)
        return;

    // find all remaining files
    set_t *scans = set_new(10, kTypeInfoPtr, rich->arena);
    size_t len = typevec_len(event->segments);
    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(event->segments, i);
        CTASSERT(segment != NULL);

        const scan_t *scan = node_get_scan(&segment->node);
        set_add(scans, scan);
    }

    const scan_t *root = node_get_scan(&event->node);

    set_iter_t iter = set_iter(scans);
    while (set_has_next(&iter))
    {
        const scan_t *scan = set_next(&iter);
        CTASSERT(scan != NULL);

        if (scan == root)
            continue;

        typevec_t *all = collect_segments(rich, event->segments, scan);
        typevec_t *merged = merge_segments(rich, all, NULL);

        size_t largest_line = longest_segment_line(merged);
        where_t where = get_first_line(merged);

        print_scan_header(rich, largest_line, where.first_line, scan);
        print_file_segments(rich, merged);
    }
}

static void print_rich_notes(rich_t *rich)
{
    CTASSERT(rich != NULL);

    const event_t *event = rich->event;
    if (event->notes == NULL)
        return;

    text_config_t config = rich->config;

    char *coloured = colour_text(rich->fmt, COLOUR_NOTE_STAR, "*");
    size_t len = vector_len(event->notes);
    for (size_t i = 0; i < len; i++)
    {
        const char *note = vector_get(event->notes, i);
        CTASSERT(note != NULL);

        io_printf(config.io, " %s %s\n", coloured, note);
    }
}

STA_DECL
void text_report_rich(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);

    file_config_t info = config.config;

    arena_t *arena = get_global_arena();

    format_context_t fmt = {
        .pallete = config.colours,
        .arena = arena,
    };

    rich_t ctx = {
        .arena = arena,
        .config = config,
        .event = event,
        .severity = get_severity(event->diagnostic, info.override_fatal),
        .file_cache = config.cache != NULL ? config.cache : cache_map_new(4, arena),
        .max_columns = info.max_columns == 0 ? 240 : info.max_columns,
        .fmt = fmt
    };

    print_report_header(&ctx, event->message);

    typevec_t *primary = collect_primary_segments(&ctx, event->segments, event);
    size_t count = typevec_len(primary);
    CT_UNUSED(count);

    print_file_header(&ctx, &event->node);
    print_file_segments(&ctx, primary);

    print_extra_files(&ctx);

    print_rich_notes(&ctx);

    if (config.cache == NULL)
        cache_map_delete(ctx.file_cache);
}
