// SPDX-License-Identifier: LGPL-3.0-only

#include "format/backtrace.h"
#include "common.h"

#include "backtrace/backtrace.h"

#include "io/io.h"

#include "arena/arena.h"

#include "std/str.h"
#include "std/typed/vector.h"

#include "base/panic.h"
#include "base/util.h"

#include "core/macros.h"

#define COLOUR_ADDR eColourRed
#define COLOUR_SYMBOL eColourBlue
#define COLOUR_INDEX eColourCyan
#define COLOUR_RECURSE eColourYellow
#define COLOUR_LINE eColourWhite

static const char *const kUnknownSymbol = "<unknown>";

typedef struct bt_report_t
{
    /// @brief memory pool
    arena_t *arena;

    /// @brief all entries
    /// @note typevec_t<entry_t>
    typevec_t *frames;
} bt_report_t;

/// @brief a single backtrace entry
typedef struct entry_t
{
    /// @brief what symbol info has been resolved by the backend?
    bt_resolve_t info;

    /// @brief the file name
    /// @note dont use this if @a info does not have @a eResolveFile
    char *file;

    /// @brief the symbol name
    /// @note dont use this if @a info does not have @a eResolveName or @a eResolveDemangledName
    char *symbol;

    /// @brief the line number
    /// @note dont use this if @a info does not have @a eResolveLine
    size_t line;

    /// @brief the address of the frame
    bt_address_t address;
} entry_t;

// state for a backtrace format run
typedef struct backtrace_t
{
    /// the user provided options
    fmt_backtrace_t options;

    /// format context
    format_context_t format_context;

    /// the collected entries from the report
    /// @note typevec_t<entry_t>
    typevec_t *entries;

    /// the collapsed frames
    /// @note typevec_t<collapsed_t>
    typevec_t *frames;

    /// the alignment of the frame index numbers
    size_t index_align;

    /// the current alignment of the symbol names
    size_t symbol_align;

    /// the length of the source root
    size_t source_root_len;

    /// arena
    arena_t *arena;
} backtrace_t;

/// @brief a single possibly collapsed frame
/// this is a span covering (first, last) * repeat frames
typedef struct collapsed_t
{
    /// @brief index of the first frame in this sequence
    unsigned first;

    /// @brief index of the last frame in this sequence
    unsigned last;

    /// @brief how many times this sequence repeats
    unsigned repeat;
} collapsed_t;

static bool is_collapsed_range(collapsed_t collapsed)
{
    return collapsed.repeat != 0;
}

static const entry_t *get_collapsed_entry(const backtrace_t *self, collapsed_t range)
{
    CTASSERTF(!is_collapsed_range(range), "range is not a single frame");

    return typevec_offset(self->entries, range.first);
}

// stacktrace collapsing

// this approach is O(n^2) but it is simple
// we scan along the trace and collect the current sequence
// we collect the current sequence into a buffer, and if we reach the end we discard it
// and start again with the next frame
// we compare the current symbol with the start of the sequence, if they match we start collecting
// we store the collapsed frames and unmatched frames in a new buffer
// the final buffer is what we print

/// @brief try and match a sequence of frames
/// scans frames starting at scan_start for the sequence of frames (seq_start, scan_start)
/// returns the number of frames matched
static unsigned match_sequence(
    const typevec_t *frames,
    unsigned scan_start,
    unsigned seq_start,
    collapsed_t *collapsed
)
{
    size_t seq_len = scan_start - seq_start;
    size_t ent_len = typevec_len(frames);

    // see how many times the sequence repeats
    // if it doesnt repeat then return false
    size_t count = 0;
    size_t j = 0;
    for (size_t i = scan_start; i < ent_len; i++)
    {
        const entry_t *a = typevec_offset(frames, i);
        const entry_t *b = typevec_offset(frames, j + seq_start);

        if (a->address != b->address)
            break;

        j += 1;

        if (j == seq_len)
        {
            count += 1;
            j = 0;
        }
    }

    if (count == 0)
        return 0; // no match

    // we have a match
    // we need to store the sequence and the repeat count

    collapsed->first = (unsigned)(scan_start);
    collapsed->last = (unsigned)(scan_start + seq_len - 1);
    collapsed->repeat = (unsigned)(count);
    return (unsigned)(count * seq_len);
}

static unsigned collapse_frame(const typevec_t *frames, unsigned start, collapsed_t *collapsed)
{
    size_t len = typevec_len(frames);
    if (start <= len)
    {
        // push the first entry
        const entry_t *head = typevec_offset(frames, start);

        for (unsigned i = start + 1; i < len; i++)
        {
            const entry_t *frame = typevec_offset(frames, i);
            CTASSERTF(frame != NULL, "entry at %u is NULL", i);

            // we might have a match
            if (frame->address == head->address)
            {
                unsigned count = match_sequence(frames, i, start, collapsed);

                // if we matched more then we're done
                if (count != 0) return count;

                // otherwise break out and push just the single entry
                break;
            }

            // we dont have a match, so start again
        }
    }

    // if we get here then no repeating sequence was matched
    collapsed->first = start;
    collapsed->last = start;
    collapsed->repeat = 0;
    return 1;
}

static typevec_t *collapse_frames(const typevec_t *frames, arena_t *arena)
{
    size_t len = typevec_len(frames);
    typevec_t *result = typevec_new(sizeof(collapsed_t), len, arena);

    for (unsigned i = 0; i < len; i++)
    {
        collapsed_t collapsed = {0};
        unsigned count = collapse_frame(frames, i, &collapsed);

        CTASSERTF(count > 0, "count of 0 at %u", i);

        typevec_push(result, &collapsed);

        // skip the frames we collapsed
        i += count - 1;
    }

    return result;
}

// text formatting

// special case recursions with only a single frame repeated to the form of
//
// symbol x repeat (file:line)
//
// all other recursions are printed as
//
// repeats N times
// | [1] symbol (file:line)
// | [2] symbol (file:line)
// | [N] symbol (file:line)

// the length of a pointer in hex, plus the 0x prefix
#define PTR_TEXT_LEN (2 + 2 * sizeof(void*))

// get the longest symbol in a sequence of entries
static size_t get_longest_symbol(const typevec_t *frames, collapsed_t range)
{
    CTASSERTF(is_collapsed_range(range), "range is not a single frame");
    size_t largest = 0;

    for (size_t i = range.first; i <= range.last; i++)
    {
        const entry_t *entry = typevec_offset(frames, i);
        CTASSERTF(entry != NULL, "entry at %zu is NULL", i);

        size_t width = ctu_strlen(entry->symbol);
        largest = CT_MAX(width, largest);
    }

    return largest;
}

typedef struct symbol_match_info_t
{
    // the largest symbol in this sequence
    size_t largest_symbol;

    // how many entries in this sequence before needing to recalculate
    // the largest symbol
    size_t count;
} symbol_match_info_t;

static symbol_match_info_t get_largest_collapsed_symbol(const typevec_t *frames, const collapsed_t *entries, size_t len)
{
    CTASSERT(entries != NULL);

    size_t largest = ctu_strlen(kUnknownSymbol);

    for (size_t i = 0; i < len; i++)
    {
        const collapsed_t *range = entries + i;
        CTASSERTF(range != NULL, "entry at %zu is NULL", i);

        if (is_collapsed_range(*range))
        {
            symbol_match_info_t info = {
                .largest_symbol = largest,
                .count = i
            };

            return info;
        }

        const entry_t *entry = typevec_offset(frames, range->first);

        size_t width = ctu_strlen(entry->symbol);
        largest = CT_MAX(width, largest);
    }

    symbol_match_info_t info = {
        .largest_symbol = largest,
        .count = len
    };
    return info;
}

static const char *get_file_path(backtrace_t *pass, const char *file)
{
    fmt_backtrace_t config = pass->options;
    const char *path = config.project_source_path;
    size_t len = pass->source_root_len;

    if (path == NULL)
        return file;

    if (str_startswith(file, path))
        return file + len;

    return file;
}

static const char *fmt_entry_location(backtrace_t *pass, const entry_t *entry)
{
    fmt_backtrace_t config = pass->options;

    bt_resolve_t resolved = entry->info;

    if (resolved & eResolveFile)
    {
        const char *file = get_file_path(pass, entry->file);

        if (resolved & eResolveLine)
        {
            where_t where = {
                .first_line = entry->line,
            };

            source_config_t source_config = {
                .context = pass->format_context,
                .colour = eColourDefault,
                .heading_style = config.header,
                .zero_indexed_lines = config.config & eBtZeroIndexedLines,
            };

            char *out = fmt_source_location(source_config, file, where);
            return str_format(pass->format_context.arena, "(%s)", out);
        }

        return file;
    }

    return colour_format(pass->format_context, COLOUR_ADDR, "0x%" BT_PRI_ADDRESS, entry->address);
}

static char *fmt_entry(backtrace_t *pass, size_t symbol_align, const entry_t *entry)
{
    fmt_backtrace_t config = pass->options;
    print_options_t options = config.options;

    bt_resolve_t resolved = entry->info;

    // we only need the @ seperator if we only have the address
    bool needs_seperator = !((resolved & eResolveFile) && (resolved & eResolveLine));
    const char *where = fmt_entry_location(pass, entry);
    const char *name = (resolved & eResolveName) ? entry->symbol : kUnknownSymbol;

    char *it = fmt_right_align(options.arena, symbol_align, "%s", name);
    char *coloured = colour_text(pass->format_context, COLOUR_SYMBOL, it);

    if (needs_seperator)
        return str_format(options.arena, "%s @ %s", coloured, where);

    return str_format(options.arena, "%s %s", coloured, where);
}

static char *fmt_index(backtrace_t *pass, size_t index)
{
    fmt_backtrace_t config = pass->options;
    print_options_t options = config.options;

    char *idx = fmt_left_align(options.arena, pass->index_align, "%zu", index);
    return colour_format(pass->format_context, COLOUR_INDEX, "[%s]", idx);
}

static void print_single_frame(backtrace_t *pass, size_t index, const entry_t *entry)
{
    fmt_backtrace_t options = pass->options;
    print_options_t base = options.options;
    char *idx = fmt_index(pass, index);
    io_printf(base.io, "%s %s\n", idx, fmt_entry(pass, pass->symbol_align, entry));
}

static void print_frame_sequence(backtrace_t *pass, size_t index, collapsed_t collapsed)
{
    fmt_backtrace_t options = pass->options;
    print_options_t base = options.options;

    size_t largest = get_longest_symbol(pass->entries, collapsed);

    char *idx = fmt_index(pass, index);
    char *coloured = colour_format(pass->format_context, COLOUR_RECURSE, "%u", collapsed.repeat);
    io_printf(base.io, "%s repeats %s times\n", idx, coloured);

    for (size_t i = collapsed.first; i <= collapsed.last; i++)
    {
        const entry_t *entry = typevec_offset(pass->entries, i);
        CTASSERTF(entry != NULL, "entry at %zu is NULL", i);

        char *it = fmt_entry(pass, largest, entry);
        char *inner = colour_format(pass->format_context, COLOUR_RECURSE, "[%zu]", i);
        io_printf(base.io, " - %s %s\n", inner, it);
    }
}

static void print_collapsed(backtrace_t *pass, size_t index, collapsed_t collapsed)
{
    if (is_collapsed_range(collapsed))
    {
        print_frame_sequence(pass, index, collapsed);
    }
    else
    {
        const entry_t *entry = get_collapsed_entry(pass, collapsed);
        print_single_frame(pass, index, entry);
    }
}

STA_DECL
void fmt_backtrace(fmt_backtrace_t fmt, bt_report_t *report)
{
    CTASSERT(report != NULL);

    print_options_t options = fmt.options;

    typevec_t *entries = collapse_frames(report->frames, options.arena);

    collapsed_t *collapsed = typevec_data(entries);
    size_t frame_count = typevec_len(entries);

    symbol_match_info_t symbol_align = get_largest_collapsed_symbol(report->frames, collapsed, frame_count);
    int align = get_num_width(frame_count);

    backtrace_t pass = {
        .options = fmt,
        .format_context = format_context_make(options),
        .entries = report->frames,
        .frames = entries,
        .index_align = align,
        .symbol_align = symbol_align.largest_symbol,
        .arena = options.arena,
    };

    const char *source_path = fmt.project_source_path;
    if (source_path)
    {
        size_t len = ctu_strlen(source_path);
        if (source_path[len - 1] != '/' && source_path[len - 1] != '\\')
            len += 1;
        pass.source_root_len = len;
    }

    for (size_t i = 0; i < frame_count; i++)
    {
        const collapsed_t *frame = typevec_offset(entries, i);
        CTASSERTF(frame != NULL, "collapsed at %zu is NULL", i);

        // this logic handles aligning symbol names on a per region basis
        // a region is defined as consecutive frames that were not recursive
        if (symbol_align.count > 0)
        {
            symbol_align.count -= 1;
        }
        else
        {
            if (!is_collapsed_range(*frame))
            {
                size_t remaining = frame_count - i;
                symbol_align = get_largest_collapsed_symbol(report->frames, frame, remaining);
                pass.symbol_align = symbol_align.largest_symbol;
            }
        }

        print_collapsed(&pass, i, *frame);
    }
}

STA_DECL
bt_report_t *bt_report_new(arena_t *arena)
{
    CTASSERT(arena != NULL);

    bt_report_t *report = ARENA_MALLOC(sizeof(bt_report_t), "bt_report", NULL, arena);
    report->arena = arena;
    report->frames = typevec_new(sizeof(entry_t), 4, arena);

    ARENA_IDENTIFY(report->frames, "entries", report, arena);

    return report;
}

static void read_stacktrace_frame(bt_address_t frame, void *user)
{
    bt_report_add(user, frame);
}

STA_DECL
bt_report_t *bt_report_collect(arena_t *arena)
{
    bt_report_t *report = bt_report_new(arena);

    bt_read(read_stacktrace_frame, report);

    return report;
}

STA_DECL
void bt_report_add(bt_report_t *report, bt_address_t frame)
{
    CTASSERT(report != NULL);

    char path[1024];
    char name[1024];

    bt_symbol_t symbol = {
        .name = text_make(name, sizeof(name)),
        .path = text_make(path, sizeof(path)),
    };
    bt_resolve_t info = bt_resolve_symbol(frame, &symbol);

    entry_t entry = {
        .info = info,
        .file = arena_strndup(symbol.path.text, symbol.path.length, report->arena),
        .symbol = arena_strndup(symbol.name.text, symbol.name.length, report->arena),
        .line = symbol.line,
        .address = frame,
    };

    typevec_push(report->frames, &entry);
}
