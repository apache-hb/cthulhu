// SPDX-License-Identifier: LGPL-3.0-only

#include "common_extra.h"

#include "memory/memory.h"
#include "format/notify.h"

#include "io/io.h"
#include "scan/node.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include "base/util.h"
#include "base/panic.h"

#define COLOUR_NOTE eColourGreen
#define COLOUR_PATH eColourBlue

typedef struct simple_t
{
    io_t *io;
    arena_t *arena;

    text_config_t text;
    file_config_t file;
    format_context_t fmt;
} simple_t;

static void print_simple_notes(simple_t *simple, const vector_t *notes)
{
    if (notes == NULL)
        return;

    char *star = colour_text(simple->fmt, COLOUR_NOTE, "*");

    size_t note_count = vector_len(notes);
    for (size_t i = 0; i < note_count; i++)
    {
        const char *note = vector_get(notes, i);
        io_printf(simple->io, "  %s note: %s\n", star, note);
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

    return ctu_strcmp(path_lhs, path_rhs);
}

static void print_segment(simple_t *simple, const segment_t *segment, size_t scan_idx, size_t segment_idx)
{
    CTASSERT(segment != NULL);

    source_config_t source_config = {
        .context = simple->fmt,
        .colour = COLOUR_PATH,
        .heading_style = eHeadingGeneric,
        .zero_indexed_lines = simple->file.zeroth_line,
    };

    const char *path = fmt_node_location(source_config, &segment->node);

    const char *msg = segment->message;

    io_printf(simple->io, " (%zu.%zu) %s: %s\n", scan_idx + 1, segment_idx + 1, path, msg);
}

static void print_segment_list(simple_t *simple, const typevec_t *segments, size_t scan_idx)
{
    CTASSERT(segments != NULL);

    size_t len = typevec_len(segments);
    for (size_t i = 0; i < len; i++)
    {
        const segment_t *segment = typevec_offset(segments, i);
        print_segment(simple, segment, scan_idx, i);
    }
}

static void print_segments(simple_t *simple, const event_t *event)
{
    // get all segments not in the same scan as the event
    // first group by segments from the primary report
    // then grouped by scanner, then alphabetical by scanner name
    // then by line number

    if (event->segments == NULL)
        return;

    size_t len = typevec_len(event->segments);

    // map_t<scan_t*, typevec_t<segment_t>>
    map_t *scans = map_optimal(len, kTypeInfoPtr, simple->arena);
    typevec_t *none = typevec_new(sizeof(segment_t), 4, simple->arena);
    typevec_t *primary = all_segments_in_scan(event->segments, &event->node, simple->arena);

    for (size_t i = 0; i < len; i++)
    {
        segment_t *segment = typevec_offset(event->segments, i);
        CTASSERT(segment != NULL);

        const scan_t *scan = node_get_scan(&segment->node);

        if (scan == node_get_scan(&event->node))
            continue;

        if (scan == NULL)
        {
            typevec_push(none, segment);
            continue;
        }

        typevec_t *segments = map_get(scans, scan);

        if (segments == NULL)
        {
            segments = typevec_new(sizeof(segment_t), 2, simple->arena);
            map_set(scans, scan, segments);
        }

        typevec_push(segments, segment);
    }

    print_segment_list(simple, primary, 0);

    typevec_t *entries = map_entries(scans);
    typevec_sort(entries, entry_cmp);

    size_t entry_count = typevec_len(entries);
    for (size_t i = 0; i < entry_count; i++)
    {
        map_entry_t *entry = typevec_offset(entries, i);
        CTASSERT(entry != NULL);

        typevec_t *segments = entry->value;
        segments_sort(segments);

        print_segment_list(simple, segments, i + 1);
    }

    print_segment_list(simple, none, entry_count + 1);
}

USE_DECL
void text_report_simple(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);
    const diagnostic_t *diagnostic = event->diagnostic;

    io_t *io = config.io;
    arena_t *arena = get_global_arena();

    format_context_t fmt = {
        .pallete = config.colours,
        .arena = arena,
    };

    simple_t simple = {
        .io = io,
        .arena = arena,
        .text = config,
        .file = config.config,
        .fmt = fmt
    };

    file_config_t cfg = config.config;
    severity_t severity = get_severity(diagnostic, cfg.override_fatal);

    const char *sev = get_severity_name(severity);
    colour_t col = get_severity_colour(severity);

    source_config_t source_config = {
        .context = fmt,
        .colour = COLOUR_PATH,
        .heading_style = eHeadingGeneric,
        .zero_indexed_lines = cfg.zeroth_line,
    };

    const char *path = fmt_node_location(source_config, &event->node);
    const char *lvl = colour_format(fmt, col, "%s: %s:", sev, diagnostic->id);

    io_printf(io, "%s %s %s:\n", path, lvl, event->message);

    print_segments(&simple, event);

    print_simple_notes(&simple, event->notes);
}
