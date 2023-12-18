#include "common.h"

#include "io/io.h"
#include "memory/memory.h"

#include "base/panic.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/str.h"

#include "std/typed/vector.h"

#include <string.h>

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

    if (!node_is_builtin(node))
    {
        line_t first_line = get_line_number(config, node);

        return format("%s:%" PRI_LINE ":%" PRI_COLUMN "", path, first_line, where.first_column);
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

line_t get_line_number(file_config_t config, const node_t *node)
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

    char *result = ctu_malloc(width + 1);
    memset(result, ' ', width);
    memcpy(result, msg, len);

    result[width] = '\0';

    ctu_free(msg);

    return result;
}

typedef struct lineinfo_t
{
    size_t offset;
    size_t length;
} lineinfo_t;

typedef struct text_cache_t
{
    io_t *io;
    text_view_t source;

    typevec_t *line_info;
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

static text_cache_t *text_cache_new(io_t *io, text_view_t source, size_t len)
{
    text_cache_t *cache = ctu_malloc(sizeof(text_cache_t));
    cache->io = io;
    cache->source = source;
    cache->line_info = typevec_new(sizeof(lineinfo_t), len);

    return cache;
}

static text_cache_t *text_cache_io(io_t *io)
{
    CTASSERT(io != NULL);

    text_view_t view = {
        .text = io_map(io),
        .size = io_size(io)
    };

    text_cache_t *cache = text_cache_new(io, view, 32);

    if (io_error(io) == 0)
        load_lineinfo(cache);

    return cache;
}

static text_cache_t *text_cache_scan(const scan_t *scan)
{
    CTASSERT(scan != NULL);

    text_view_t view = scan_source(scan);

    text_cache_t *cache = text_cache_new(NULL, view, 32);

    load_lineinfo(cache);

    return cache;
}

static void text_cache_delete(text_cache_t *cache)
{
    CTASSERT(cache != NULL);

    if (cache->io != NULL) io_close(cache->io);
    typevec_delete(cache->line_info);
    ctu_free(cache);
}

static bool cache_is_valid(text_cache_t *cache)
{
    CTASSERT(cache != NULL);

    return cache->io != NULL && io_error(cache->io) == 0;
}

cache_map_t *cache_map_new(size_t size)
{
    return map_optimal(size);
}

void cache_map_delete(cache_map_t *map)
{
    CTASSERT(map != NULL);

    map_iter_t iter = map_iter(map);
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

    text_cache_t *cache = map_get(map, path);
    if (cache != NULL && cache_is_valid(cache)) return cache;

    io_t *io = io_file(path, eAccessRead | eAccessText);
    text_cache_t *text = text_cache_io(io);

    // always insert the cache, even if it is invalid.
    // this way we avoid trying to open the file again
    map_set(map, path, text);
    if (cache_is_valid(text)) return text;

    return NULL;
}

text_cache_t *cache_emplace_scan(cache_map_t *map, const scan_t *scan)
{
    CTASSERT(map != NULL);
    CTASSERT(scan != NULL);

    text_cache_t *cache = map_get_ptr(map, scan);
    if (cache != NULL && cache_is_valid(cache)) return cache;

    // scan caches will never be invalid, so we can just insert them
    text_cache_t *text = text_cache_scan(scan);
    map_set_ptr(map, scan, text);

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

#if 0

typedef struct lineinfo_t
{
    size_t offset;
    size_t length;
} lineinfo_t;

typedef struct source_line_t
{
    char *text;

    // typevec_t<segment_t>
    typevec_t *segments;
} source_line_t;

typedef struct sparse_text_t
{
    const scan_t *scan;
    text_view_t source;

    // typevec_t<lineinfo_t>
    typevec_t *line_info;

    // map_t<line_t, source_line_t *>
    map_t *line_cache;
} sparse_text_t;

typedef struct sparse_report_t
{
    const scan_t *primary;

    // map_t<const scan_t*, sparse_text_t*>
    map_t *files;
} sparse_report_t;

// load the start and length of each line in the file
static void load_lineinfo(sparse_text_t *text)
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

static text_view_t get_source(const node_t *node)
{
    const scan_t *scan = node_get_scan(node);
    const char *text = scan_text(scan);
    size_t size = scan_size(scan);

    text_view_t source = {
        .text = text,
        .size = size,
    };

    return source;
}

static sparse_text_t *sparse_text_new(const node_t *node)
{
    CTASSERT(node != NULL);

    const scan_t *scan = node_get_scan(node);
    text_view_t source = get_source(node);

    sparse_text_t *sparse = MEM_ALLOC(sizeof(sparse_text_t), "sparse_text", NULL);
    sparse->scan = scan;
    sparse->source = source;
    sparse->line_info = typevec_new(sizeof(lineinfo_t), 32);

    load_lineinfo(sparse);

    size_t line_count = typevec_len(sparse->line_info);
    sparse->line_cache = map_optimal(line_count);

    return sparse;
}

static source_line_t *sparse_get_cached_line(sparse_text_t *text, line_t line)
{
    CTASSERT(text != NULL);

    // add 1 because map_get_ptr uses 0 as a sentinel
    line += 1;

    return map_get_ptr(text->line_cache, (void*)(uintptr_t)line);
}

static void sparse_set_cached_line(sparse_text_t *text, line_t line, source_line_t *cached)
{
    CTASSERT(text != NULL);
    CTASSERT(cached != NULL);

    line += 1;

    map_set_ptr(text->line_cache, (void*)(uintptr_t)line, cached);
}

static void sparse_cache_line(sparse_text_t *text, line_t line, const segment_t *segment)
{
    // is this line already cached?
    source_line_t *cached = sparse_get_cached_line(text, line);
    if (cached != NULL)
    {
        typevec_push(cached->segments, segment);
        return;
    }

    // find the line info for this line
    size_t len = typevec_len(text->line_info);
    if (len <= line) return; // line is out of bounds. why?

    lineinfo_t *info = typevec_offset(text->line_info, line);

    // copy the line into a buffer
    char *buffer = ctu_strndup(text->source.text + info->offset, info->length);

    source_line_t *source_line = MEM_ALLOC(sizeof(source_line_t), "source_line", text);
    source_line->text = buffer;
    source_line->segments = typevec_init(sizeof(segment_t), segment);

    sparse_set_cached_line(text, line, source_line);
}

static void sparse_cache_lines(sparse_text_t *text, const segment_t *segment)
{
    CTASSERT(text != NULL);
    CTASSERT(segment != NULL);

    const node_t *node = segment->node;
    where_t where = node_get_location(node);

    for (line_t line = where.first_line; line <= where.last_line; line++)
    {
        sparse_cache_line(text, line, segment);
    }
}

sparse_report_t *sparse_report_new(const event_t *event)
{
    CTASSERT(event != NULL);

    const scan_t *primary_scan = node_get_scan(event->node);
    size_t len = typevec_len(event->segments);

    sparse_report_t *report = MEM_ALLOC(sizeof(sparse_report_t), "sparse_report", NULL);
    report->primary = primary_scan;
    report->files = map_optimal(len);

    MEM_IDENTIFY(report->files, "sparse_files", report);

    sparse_text_t *primary = sparse_text_new(event->node);
    map_set_ptr(report->files, primary_scan, primary);

    for (size_t i = 0; i < len; i++)
    {
        segment_t *segment = typevec_offset(event->segments, i);
        CTASSERT(segment != NULL);

        const scan_t *scan = node_get_scan(segment->node);
        if (scan == primary_scan) continue;

        sparse_text_t *sparse = map_get_ptr(report->files, scan);
        if (sparse == NULL)
        {
            sparse = sparse_text_new(segment->node);
            map_set_ptr(report->files, scan, sparse);
        }

        sparse_cache_lines(sparse, segment);
    }

    return report;
}

static int filename_cmp(const void *lhs, const void *rhs)
{
    const sparse_text_t *text_lhs = lhs;
    const sparse_text_t *text_rhs = rhs;

    const scan_t *scan_lhs = text_lhs->scan;
    const scan_t *scan_rhs = text_rhs->scan;

    const char *path_lhs = scan_path(scan_lhs);
    const char *path_rhs = scan_path(scan_rhs);

    return strcmp(path_lhs, path_rhs);
}

vector_t *sparse_report_get_files(const sparse_report_t *report)
{
    CTASSERT(report != NULL);

    vector_t *files = vector_new(4);

    sparse_text_t *primary = map_get_ptr(report->files, report->primary);
    vector_push(&files, primary);

    vector_t *values = map_values(report->files);
    vector_sort(values, filename_cmp);
    vector_append(&files, values);

    return files;
}

size_t sparse_text_count(const sparse_text_t *text)
{
    CTASSERT(text != NULL);

    return map_count(text->line_cache);
}

#endif
