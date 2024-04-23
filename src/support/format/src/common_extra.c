// SPDX-License-Identifier: LGPL-3.0-only

#include "common_extra.h"

#include "base/util.h"
#include "core/macros.h"
#include "io/io.h"
#include "memory/memory.h"

#include "base/panic.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/set.h"
#include "std/str.h"
#include "arena/arena.h"

#include "std/typed/vector.h"
#include "std/vector.h"

#include <ctype.h>

#define COLOUR_UNKNOWN eColourMagenta
#define COLOUR_SORRY eColourMagenta
#define COLOUR_INTERNAL eColourCyan
#define COLOUR_FATAL eColourRed
#define COLOUR_WARN eColourYellow
#define COLOUR_INFO eColourGreen
#define COLOUR_DEBUG eColourGreen

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
    case eSeveritySorry: return COLOUR_SORRY;
    case eSeverityInternal: return COLOUR_INTERNAL;
    case eSeverityFatal: return COLOUR_FATAL;
    case eSeverityWarn: return COLOUR_WARN;
    case eSeverityInfo:
    case eSeverityDebug: return COLOUR_INFO;
    default: return COLOUR_UNKNOWN;
    }
}

// assumes all segments are in the same file
static int segment_cmp(const void *lhs, const void *rhs)
{
    const segment_t *seg_lhs = lhs;
    const segment_t *seg_rhs = rhs;

    const scan_t *scan_lhs = node_get_scan(&seg_lhs->node);
    const scan_t *scan_rhs = node_get_scan(&seg_rhs->node);

    CTASSERTF(scan_lhs == scan_rhs, "segments must be in the same scan (%s and %s)",
              scan_path(scan_lhs), scan_path(scan_rhs));

    where_t where_lhs = node_get_location(&seg_lhs->node);
    where_t where_rhs = node_get_location(&seg_rhs->node);

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

typevec_t *all_segments_in_scan(const typevec_t *segments, const node_t *node, arena_t *arena)
{
    CTASSERT(segments != NULL);
    CTASSERT(node != NULL);

    const scan_t *scan = node_get_scan(node);

    size_t len = typevec_len(segments);
    typevec_t *result = typevec_new(sizeof(segment_t), len, arena);

    for (size_t i = 0; i < len; i++)
    {
        segment_t *segment = typevec_offset(segments, i);
        CTASSERT(segment != NULL);

        const scan_t *other = node_get_scan(&segment->node);
        if (other != scan) continue;

        typevec_push(result, segment);
    }

    segments_sort(result);

    return result;
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
    const char *end = start + text->source.length;

    while (start < end)
    {
        size_t i = str_find(start, "\n");
        const char *next = (i == SIZE_MAX) ? end : start + i;

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
    text_cache_t *cache = ARENA_MALLOC(sizeof(text_cache_t), "text_cache", NULL, arena);
    cache->arena = arena;
    cache->io = io;
    cache->source = source;
    cache->line_info = typevec_new(sizeof(lineinfo_t), len, arena);
    cache->cached_lines = map_optimal(len, kTypeInfoString, arena);

    return cache;
}

static text_view_t get_io_view(io_t *io)
{
    if (io_error(io) != 0)
    {
        return text_view_from("");
    }

    const void *memory = io_map(io, eOsProtectRead);
    size_t size = io_size(io);

    CTASSERTF(memory != NULL, "io_map(%s) failed", io_name(io));

    return text_view_make(memory, size);
}

static text_cache_t *text_cache_io(io_t *io, arena_t *arena)
{
    CTASSERT(io != NULL);

    text_view_t view = get_io_view(io);

    text_cache_t *cache = text_cache_new(io, view, 32, arena);

    if (io_error(io) == 0) load_lineinfo(cache);

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
    arena_free(cache, sizeof(text_cache_t), cache->arena);
}

static bool cache_is_valid(text_cache_t *cache)
{
    CTASSERT(cache != NULL);

    return cache->io == NULL || io_error(cache->io) == 0;
}

cache_map_t *cache_map_new(size_t size, arena_t *arena)
{
    CTASSERT(arena != NULL);

    cache_map_t *data = ARENA_MALLOC(sizeof(cache_map_t), "cache_map", NULL, arena);
    data->arena = arena;
    data->map = map_optimal(size, kTypeInfoString, arena);

    return data;
}

void cache_map_delete(cache_map_t *map)
{
    CTASSERT(map != NULL);

    map_iter_t iter = map_iter(map->map);
    const char *key = NULL;
    text_cache_t *cache = NULL;
    while (CTU_MAP_NEXT(&iter, &key, &cache))
    {
        text_cache_delete(cache);
    }
}

text_cache_t *cache_emplace_file(cache_map_t *map, const char *path)
{
    CTASSERT(map != NULL);
    CTASSERT(path != NULL);

    // TODO: is using the name stable?
    text_cache_t *cache = map_get(map->map, path);
    if (cache != NULL && cache_is_valid(cache)) return cache;

    io_t *io = io_file(path, eOsAccessRead, map->arena);
    text_cache_t *text = text_cache_io(io, map->arena);

    // always insert the cache, even if it is invalid.
    // this way we avoid trying to open the file again
    map_set(map->map, path, text);
    if (cache_is_valid(text)) return text;

    return NULL;
}

text_cache_t *cache_emplace_scan(cache_map_t *map, const scan_t *scan)
{
    CTASSERT(map != NULL);
    CTASSERT(scan != NULL);

    text_cache_t *cache = map_get(map->map, scan);
    if (cache != NULL && cache_is_valid(cache)) return cache;

    // scan caches will never be invalid, so we can just insert them
    text_cache_t *text = text_cache_scan(scan, map->arena);
    map_set(map->map, scan, text);

    return text;
}

text_view_t cache_get_line(text_cache_t *cache, size_t line)
{
    CTASSERT(cache != NULL);

    bool has_line = typevec_len(cache->line_info) > line;

    if (!has_line)
    {
        // line is out of bounds. why?
        return text_view_make("", 0);
    }

    lineinfo_t *info = typevec_offset(cache->line_info, line);

    return text_view_make(cache->source.text + info->offset, info->length);
}

size_t cache_count_lines(text_cache_t *cache)
{
    CTASSERT(cache != NULL);

    return typevec_len(cache->line_info);
}

static const char *const kHexChars = "0123456789abcdef";

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
    buf[1] = kHexChars[c >> 4];
    buf[2] = kHexChars[c & 0xf];
    buf[3] = '\0';

    return true;
}

text_t cache_escape_line(text_cache_t *cache, size_t line, const colour_pallete_t *colours,
                         size_t column_limit)
{
    CTASSERT(colours != NULL);

    text_t *cached = map_get(cache->cached_lines, (void *)(uintptr_t)(line + 1));
    if (cached != NULL) return *cached;

    text_view_t view = cache_get_line(cache, line);

    typevec_t *result = typevec_new(sizeof(char), view.length * 2, cache->arena);

    // we use a temporary buffer to format into because this is a hot path
    // when reporting very long lines, snprintf is too slow

    // 8 characters is enough for any 1 byte hex value
    char buffer[8] = "";
    bool in_colour = false;
    size_t used = column_limit;
    for (size_t i = 0; i < view.length; i++)
    {
        char c = view.text[i];
        bool is_notprint = get_escaped_char(buffer, c);
        if (is_notprint && !in_colour)
        {
            const char *colour = colour_get(colours, COLOUR_UNKNOWN);
            typevec_append(result, colour, ctu_strlen(colour));
            in_colour = true;
        }
        else if (!is_notprint && in_colour)
        {
            const char *reset = colour_reset(colours);
            typevec_append(result, reset, ctu_strlen(reset));
            in_colour = false;
        }

        typevec_append(result, buffer, ctu_strlen(buffer));

        if (0 >= --used)
        {
            break;
        }
    }

    text_t *ptr = ARENA_MALLOC(sizeof(text_t), "text", NULL, cache->arena);
    ptr->text = typevec_data(result);
    ptr->length = typevec_len(result);

    map_set(cache->cached_lines, (void *)(uintptr_t)(line + 1), ptr);

    return *ptr;
}

static bool set_has_option(set_t *set, const diagnostic_t *diag)
{
    if (set == NULL) return false;

    return set_contains(set, diag);
}

STA_DECL
int text_report(typevec_t *events, report_config_t config, const char *title)
{
    CTASSERT(events != NULL);
    CTASSERT(title != NULL);

    arena_t *arena = get_global_arena();
    size_t error_budget = config.max_errors == 0 ? 20 : config.max_errors;
    size_t warn_budget = config.max_warnings == 0 ? 20 : config.max_warnings;

    cache_map_t *cache = cache_map_new(32, arena);

    text_format_t fmt = config.report_format;
    text_config_t text = config.text_config;

    text.cache = cache;

    size_t len = typevec_len(events);
    void (*fn)(text_config_t, const event_t *) = fmt == eTextComplex ? text_report_rich
                                                                     : text_report_simple;

    int result = CT_EXIT_OK;
    size_t warning_count = 0;
    size_t error_count = 0;
    size_t bug_count = 0;


    for (size_t i = 0; i < len; i++)
    {
        event_t *event = typevec_offset(events, i);
        const diagnostic_t *diag = event->diagnostic;
        if (set_has_option(config.ignore_warnings, diag))
        {
            continue;
        }

        switch (diag->severity)
        {
        case eSeverityWarn:
            warn_budget -= 1;
            if (warn_budget == 0) continue;
            break;

        case eSeverityFatal:
            error_budget -= 1;
            if (error_budget == 0) continue;
            break;

        default: break;
        }

        text.config.override_fatal = set_has_option(config.error_warnings, diag);

        fn(text, event);

        switch (diag->severity)
        {
        case eSeverityFatal:
            result = CT_MAX(result, CT_EXIT_ERROR);
            error_count += 1;
            break;
        case eSeverityInternal:
        case eSeveritySorry:
            result = CT_MAX(result, CT_EXIT_INTERNAL);
            bug_count += 1;
            break;
        case eSeverityWarn:
            warning_count += 1;
            /* fallthrough */
        default: break;
        }
    }

    io_t *io = text.io;

    format_context_t ctx = {
        .arena = arena,
        .pallete = text.colours,
    };

    if (result != CT_EXIT_OK)
    {
        io_printf(io, "compilation failed during stage: %s\n", title);
        vector_t *parts = vector_new(3, arena);
        if (error_count > 0)
        {
            char *colour_err = colour_format(ctx, COLOUR_FATAL, "%zu errors", error_count);
            vector_push(&parts, colour_err);
        }

        if (warning_count > 0)
        {
            char *colour_warn = colour_format(ctx, COLOUR_WARN, "%zu warnings", warning_count);
            vector_push(&parts, colour_warn);
        }

        if (bug_count > 0)
        {
            char *colour_bug = colour_format(ctx, COLOUR_INTERNAL, "%zu bugs", bug_count);
            vector_push(&parts, colour_bug);
        }

        char *msg = str_join(", ", parts, arena);
        io_printf(io, "  %s\n", msg);
    }
    else if (warning_count > 0)
    {
        io_printf(io, "compilation succeeded with warnings during stage: %s\n", title);
        char *colour_warn = colour_format(ctx, COLOUR_WARN, "%zu warnings", warning_count);
        io_printf(io, "  %s\n", colour_warn);
    }

    return result;
}

char *fmt_node_location(source_config_t config, const node_t *node)
{
    const scan_t *scan = node_get_scan(node);
    const char *path = scan_is_builtin(scan) ? NULL : scan_path(scan);
    where_t where = node_get_location(node);

    return fmt_source_location(config, path, where);
}
