#include "common.h"

#include "core/macros.h"
#include "core/text.h"
#include "io/io.h"
#include "memory/memory.h"

#include "base/panic.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/str.h"
#include "std/set.h"

#include "std/typed/vector.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

severity_t get_severity(const diagnostic_t *diag, bool override_fatal)
{
    severity_t severity = diag->severity;
    if (override_fatal && severity == eSeverityWarn) return eSeverityFatal;

    return severity;
}

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
    case eSeverityInfo:
    case eSeverityDebug: return eColourGreen;
    default: return eColourDefault;
    }
}

const char *get_scan_name(const node_t *node)
{
    if (node_is_builtin(node)) return "builtin";

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

    CTASSERTF(scan_lhs == scan_rhs, "segments must be in the same scan (%s and %s)",
              scan_path(scan_lhs), scan_path(scan_rhs));

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
    typevec_t *result = typevec_new(sizeof(segment_t), len, get_global_arena());

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

    if (!node_is_builtin(node))
    {
        size_t first_line = get_line_number(config, node);

        return format("%s:%zu:%" PRI_COLUMN "", path, first_line, where.first_column);
    }
    else
    {
        return format("<%s>", path);
    }
}

char *fmt_coloured(const text_colour_t *colours, colour_t idx, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = vformat(fmt, args);
    va_end(args);

    return format("%s%s%s", colour_get(colours, idx), msg, colour_reset(colours));
}

size_t get_line_number(file_config_t config, const node_t *node)
{
    where_t where = node_get_location(node);
    if (config.zeroth_line) return where.first_line;

    return where.first_line + 1;
}

bool node_has_line(const node_t *node)
{
    return !node_is_builtin(node);
}

size_t get_offset_line(file_config_t config, size_t line)
{
    // if the first line is 0, then we don't need to do anything
    if (config.zeroth_line) return line;

    // otherwise, we need to subtract 1 from the line number
    return line == 0 ? line : line - 1;
}

size_t get_num_width(size_t num)
{
    size_t width = 0;

    while (num > 0)
    {
        num /= 10;
        width++;
    }

    return width;
}

char *fmt_align(size_t width, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *msg = vformat(fmt, args);
    va_end(args);

    size_t len = strlen(msg);
    if (len >= width) return msg;

    arena_t *arena = get_global_arena();
    size_t size = width - 1;
    char *result = ARENA_MALLOC(arena, size, "align", NULL);
    memset(result, ' ', width);
    memcpy(result, msg, len);

    result[width] = '\0';

    arena_free(msg, size, arena);

    return result;
}

typedef struct cache_map_t
{
    arena_t *arena;
    map_t *map;
} cache_map_t;

typedef struct lineinfo_t
{
    size_t offset;
    size_t length;
} lineinfo_t;

typedef struct text_cache_t
{
    arena_t *arena;

    io_t *io;
    text_view_t source;

    typevec_t *line_info;

    map_t *cached_lines;
} text_cache_t;

// load the start and length of each line in the file
static void load_lineinfo(text_cache_t *text)
{
    size_t offset = 0;

    const char *start = text->source.text;
    const char *end = start + text->source.size;

    while (start < end)
    {
        const char *next = strchr(start, '\n');
        if (next == NULL) next = end;

        lineinfo_t info = {
            .offset = offset,
            .length = next - start,
        };

        typevec_push(text->line_info, &info);

        offset += info.length + 1;
        start = next + 1;
    }
}

static text_cache_t *text_cache_new(io_t *io, text_view_t source, size_t len, arena_t *arena)
{
    text_cache_t *cache = ARENA_MALLOC(arena, sizeof(text_cache_t), "text_cache", NULL);
    cache->arena = arena;
    cache->io = io;
    cache->source = source;
    cache->line_info = typevec_new(sizeof(lineinfo_t), len, arena);
    cache->cached_lines = map_optimal(len);

    return cache;
}

static text_view_t get_io_view(io_t *io)
{
    if (io_error(io) != 0) 
    {
        text_view_t view = {
            .text = "",
            .size = 0
        };

        return view;
    }

    text_view_t view = {
        .text = io_map(io),
        .size = io_size(io)
    };

    return view;
}

static text_cache_t *text_cache_io(io_t *io, arena_t *arena)
{
    CTASSERT(io != NULL);

    text_view_t view = get_io_view(io);

    text_cache_t *cache = text_cache_new(io, view, 32, arena);

    if (io_error(io) == 0)
        load_lineinfo(cache);

    return cache;
}

static text_cache_t *text_cache_scan(const scan_t *scan, arena_t *arena)
{
    CTASSERT(scan != NULL);

    text_view_t view = scan_source(scan);

    text_cache_t *cache = text_cache_new(NULL, view, 32, arena);

    load_lineinfo(cache);

    return cache;
}

static void text_cache_delete(text_cache_t *cache)
{
    CTASSERT(cache != NULL);

    if (cache->io != NULL) io_close(cache->io);
    typevec_delete(cache->line_info);
    arena_free(cache, sizeof(text_cache_t), cache->arena);
}

static bool cache_is_valid(text_cache_t *cache)
{
    CTASSERT(cache != NULL);

    return cache->io == NULL || io_error(cache->io) == 0;
}

cache_map_t *cache_map_new(size_t size)
{
    arena_t *arena = get_global_arena();

    cache_map_t *data = ARENA_MALLOC(arena, sizeof(cache_map_t), "cache_map", NULL);
    data->arena = arena;
    data->map = map_optimal(size);

    return data;
}

void cache_map_delete(cache_map_t *map)
{
    CTASSERT(map != NULL);

    map_iter_t iter = map_iter(map->map);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        text_cache_t *cache = entry.value;
        text_cache_delete(cache);
    }
}

text_cache_t *cache_emplace_file(cache_map_t *map, const char *path)
{
    CTASSERT(map != NULL);
    CTASSERT(path != NULL);

    // TODO: is using the name stable?
    text_cache_t *cache = map_get_ptr(map->map, path);
    if (cache != NULL && cache_is_valid(cache)) return cache;

    io_t *io = io_file(path, eAccessRead | eAccessText, map->arena);
    text_cache_t *text = text_cache_io(io, map->arena);

    // always insert the cache, even if it is invalid.
    // this way we avoid trying to open the file again
    map_set_ptr(map->map, path, text);
    if (cache_is_valid(text)) return text;

    return NULL;
}

text_cache_t *cache_emplace_scan(cache_map_t *map, const scan_t *scan)
{
    CTASSERT(map != NULL);
    CTASSERT(scan != NULL);

    text_cache_t *cache = map_get_ptr(map->map, scan);
    if (cache != NULL && cache_is_valid(cache)) return cache;

    // scan caches will never be invalid, so we can just insert them
    text_cache_t *text = text_cache_scan(scan, map->arena);
    map_set_ptr(map->map, scan, text);

    return text;
}

text_view_t cache_get_line(text_cache_t *cache, size_t line)
{
    CTASSERT(cache != NULL);

    bool has_line = typevec_len(cache->line_info) > line;

    if (!has_line)
    {
        text_view_t view = {
            .text = "",
            .size = 0
        };

        return view; // line is out of bounds. why?
    }

    lineinfo_t *info = typevec_offset(cache->line_info, line);

    text_view_t view = {
        .text = cache->source.text + info->offset,
        .size = info->length
    };

    return view;
}

size_t cache_count_lines(text_cache_t *cache)
{
    CTASSERT(cache != NULL);

    return typevec_len(cache->line_info);
}

static bool get_escaped_char(char *buf, char c)
{
    if (isprint(c))
    {
        buf[0] = c;
        buf[1] = '\0';
        return false;
    }

    // print 2 byte hex value, we need to do this manually rather than using snprintf
    // because snprintf is too slow and for some ungodly reason this is a hot path
    buf[0] = '\\';
    buf[1] = "0123456789abcdef"[c >> 4];
    buf[2] = "0123456789abcdef"[c & 0xf];
    buf[3] = '\0';

    return true;
}

text_t cache_escape_line(text_cache_t *cache, size_t line, const text_colour_t *colours, size_t column_limit)
{
    CTASSERT(colours != NULL);

    text_t *cached = map_get_ptr(cache->cached_lines, (void*)(uintptr_t)(line + 1));
    if (cached != NULL) return *cached;

    text_view_t view = cache_get_line(cache, line);

    typevec_t *result = typevec_new(sizeof(char), view.size * 2, cache->arena);

    char buffer[8] = "";
    bool in_colour = false;
    size_t used = column_limit;
    for (size_t i = 0; i < view.size; i++)
    {
        char c = view.text[i];
        bool is_notprint = get_escaped_char(buffer, c);
        if (is_notprint && !in_colour)
        {
            const char *colour = colour_get(colours, eColourMagenta);
            typevec_append(result, colour, strlen(colour));
            in_colour = true;
        }
        else if (!is_notprint && in_colour)
        {
            const char *reset = colour_reset(colours);
            typevec_append(result, reset, strlen(reset));
            in_colour = false;
        }

        typevec_append(result, buffer, strlen(buffer));

        if (0 >= --used)
        {
            break;
        }
    }

    text_t *ptr = ARENA_MALLOC(cache->arena, sizeof(text_t), "text", NULL);
    ptr->text = typevec_data(result);
    ptr->size = typevec_len(result);

    map_set_ptr(cache->cached_lines, (void*)(uintptr_t)(line + 1), ptr);

    return *ptr;
}

static bool events_equal(const event_t *lhs, const event_t *rhs)
{
    if (lhs == rhs) return true;
    if (lhs == NULL || rhs == NULL) return false;

    if (lhs->diagnostic == rhs->diagnostic) return true;
    if (str_equal(lhs->message, rhs->message)) return true;

    return false;
}

static bool set_has_option(set_t *set, const diagnostic_t *diag)
{
    if (set == NULL) return false;

    return set_contains_ptr(set, diag);
}

USE_DECL
int text_report(typevec_t *events, report_config_t config, const char *title)
{
    CTASSERT(events != NULL);
    CTASSERT(title != NULL);

    size_t error_budget = config.max_errors == 0 ? 20 : config.max_errors;
    size_t warn_budget = config.max_warnings == 0 ? 20 : config.max_warnings;

    cache_map_t *cache = cache_map_new(32);

    text_format_t fmt = config.report_format;
    text_config_t text = config.text_config;

    text.cache = cache;

    size_t len = typevec_len(events);
    void (*fn)(text_config_t, const event_t*) = fmt == eTextComplex ? text_report_rich : text_report_simple;

    int result = EXIT_OK;
    size_t warning_count = 0;
    size_t error_count = 0;
    size_t bug_count = 0;

    const event_t *prev = NULL;
    size_t repeat = 0;

    for (size_t i = 0; i < len; i++)
    {
        event_t *event = typevec_offset(events, i);
        const diagnostic_t *diag = event->diagnostic;
        if (set_has_option(config.ignore_warnings, diag))
        {
            continue;
        }

        if (!events_equal(event, prev))
        {
            // merge consecutive events with the same message
            if (repeat > 0)
            {
                event->message = format("%s (repeated %zu times)", event->message, repeat);
            }

            switch (diag->severity) {
            case eSeverityWarn:
                warn_budget -= 1;
                if (warn_budget == 0) continue;
                break;

            case eSeverityFatal:
                error_budget -= 1;
                if (error_budget == 0) continue;
                break;

            default:
                break;
            }

            text.config.override_fatal = set_has_option(config.error_warnings, diag);

            fn(text, event);
            prev = event;
        }
        else
        {
            repeat += 1;
        }

        switch (diag->severity)
        {
        case eSeverityFatal:
            result = MAX(result, EXIT_ERROR);
            error_count += 1;
            break;
        case eSeverityInternal:
        case eSeveritySorry:
            result = MAX(result, EXIT_INTERNAL);
            bug_count += 1;
            break;
        case eSeverityWarn:
            warning_count += 1;
            /* fallthrough */
        default:
            break;
        }
    }

    io_t *io = text.io;

    if (result != EXIT_OK)
    {
        io_printf(io, "compilation failed during stage: %s\n", title);
        char *colour_err = fmt_coloured(text.colours, eColourRed, "%zu errors", error_count);
        char *colour_warn = fmt_coloured(text.colours, eColourYellow, "%zu warnings", warning_count);
        char *colour_bug = fmt_coloured(text.colours, eColourMagenta, "%zu bugs", bug_count);
        io_printf(io, "  %s, %s, %s\n", colour_err, colour_warn, colour_bug);
    }
    else if (warning_count > 0)
    {
        io_printf(io, "compilation succeeded with warnings during stage: %s\n", title);
        char *colour_warn = fmt_coloured(text.colours, eColourYellow, "%zu warnings", warning_count);
        io_printf(io, "  %s\n", colour_warn);
    }

    return result;
}
