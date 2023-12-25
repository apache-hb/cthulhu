#include "common.h"

#include "memory/memory.h"
#include "notify/text.h"

#include "base/panic.h"
#include "io/io.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include <string.h>

static void print_notes(text_config_t config, const vector_t *notes)
{
    if (notes == NULL)
        return;

    char *star = fmt_coloured(config.colours, eColourGreen, "*");

    size_t note_count = vector_len(notes);
    for (size_t i = 0; i < note_count; i++)
    {
        const char *note = vector_get(notes, i);
        io_printf(config.io, "  %s note: %s\n", star, note);
    }
}

// first round of sorting
static int entry_cmp(const void *lhs, const void *rhs)
{
    const map_entry_t *entry_lhs = lhs;
    const map_entry_t *entry_rhs = rhs;

    const scan_t *scan_lhs = entry_lhs->key;
    const scan_t *scan_rhs = entry_rhs->key;

    if (scan_lhs == NULL)
        return -1;
    if (scan_rhs == NULL)
        return 1;

    const char *path_lhs = scan_path(scan_lhs);
    const char *path_rhs = scan_path(scan_rhs);

    return strcmp(path_lhs, path_rhs);
}

static void print_segment(text_config_t config, const segment_t *segment, size_t scan_idx, size_t segment_idx)
{
    CTASSERT(segment != NULL);

    const char *msg = segment->message;
    const char *path = fmt_node(config.config, segment->node);
    const char *colour = fmt_coloured(config.colours, eColourBlue, "%s:", path);

    io_printf(config.io, " (%zu.%zu) %s %s\n", scan_idx + 1, segment_idx + 1, colour, msg);
}

static void print_segment_list(text_config_t config, const typevec_t *segments, size_t scan_idx)
{
    CTASSERT(segments != NULL);

    size_t len = typevec_len(segments);
    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(segments, i);
        print_segment(config, segment, scan_idx, i);
    }
}

static void print_segments(text_config_t config, const event_t *event)
{
    // get all segments not in the same scan as the event
    // first group by segments from the primary report
    // then grouped by scanner, then alphabetical by scanner name
    // then by line number

    if (event->segments == NULL)
        return;

    size_t len = typevec_len(event->segments);
    arena_t *arena = get_global_arena();

    // map_t<scan_t*, typevec_t<segment_t>>
    map_t *scans = map_optimal(len);
    typevec_t *none = typevec_new(sizeof(segment_t), 4, arena);
    typevec_t *primary = all_segments_in_scan(event->segments, event->node);

    for (size_t i = 0; i < len; i++)
    {
        segment_t *segment = typevec_offset(event->segments, i);
        CTASSERT(segment != NULL);

        const scan_t *scan = node_get_scan(segment->node);

        if (scan == node_get_scan(event->node))
            continue;

        if (scan == NULL)
        {
            typevec_push(none, segment);
            continue;
        }

        typevec_t *segments = map_get_ptr(scans, scan);

        if (segments == NULL)
        {
            segments = typevec_new(sizeof(segment_t), 2, arena);
            map_set_ptr(scans, scan, segments);
        }

        typevec_push(segments, segment);
    }

    print_segment_list(config, primary, 0);

    typevec_t *entries = map_entries(scans);
    typevec_sort(entries, entry_cmp);

    size_t entry_count = typevec_len(entries);
    for (size_t i = 0; i < entry_count; i++)
    {
        map_entry_t *entry = typevec_offset(entries, i);
        CTASSERT(entry != NULL);

        typevec_t *segments = entry->value;
        segments_sort(segments);

        print_segment_list(config, segments, i + 1);
    }

    print_segment_list(config, none, entry_count + 1);
}

static char *fmt_path(file_config_t config, const node_t *node)
{
    if (!node_has_line(node))
    {
        return fmt_node(config, node);
    }

    where_t where = node_get_location(node);
    size_t line = get_offset_line(config, where.first_line);

    const char *path = get_scan_name(node);

    return format("%s(%zu)", path, line);
}

USE_DECL
void text_report_simple(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);
    const diagnostic_t *diagnostic = event->diagnostic;

    file_config_t cfg = config.config;
    severity_t severity = get_severity(diagnostic, cfg.override_fatal);

    const char *sev = get_severity_name(severity);
    colour_t col = get_severity_colour(severity);

    const char *path = fmt_path(config.config, event->node);
    const char *lvl = fmt_coloured(config.colours, col, "%s %s:", sev, diagnostic->id);

    const char *path_coloured = fmt_coloured(config.colours, eColourBlue, "%s:", path);

    io_printf(config.io, "%s %s %s:\n", path_coloured, lvl, event->message);

    print_segments(config, event);

    print_notes(config, event->notes);
}
