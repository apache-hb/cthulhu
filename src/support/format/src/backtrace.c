#include "base/panic.h"
#include "base/text.h"
#include "common.h"

#include "core/macros.h"
#include "io/io.h"
#include "arena/arena.h"
#include "format/notify.h"

#include "backtrace/backtrace.h"
#include "format/backtrace.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"

#include <string.h>

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
    typevec_t *entries;
} bt_report_t;

/// @brief a single backtrace entry
typedef struct entry_t
{
    /// @brief what symbol info has been resolved by the backend?
    frame_resolve_t info;

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
    print_backtrace_t options;

    /// format context
    format_context_t format_context;

    /// the collapsed frames
    typevec_t *frames;

    size_t index_align;
    size_t symbol_align;
} backtrace_t;

typedef struct collapsed_t
{
    // the sequence of frames
    typevec_t *sequence;

    // how many times the sequence repeats
    size_t repeat;

    // a single entry if collapsed is empty
    entry_t *entry;
} collapsed_t;

// stacktrace collapsing

// this approach is O(n^2) but it is simple
// we scan along the trace and collect the current sequence
// we collect the current sequence into a buffer, and if we reach the end we discard it
// and start again with the next frame
// we compare the current symbol with the start of the sequence, if they match we start collecting
// we store the collapsed frames and unmatched frames in a new buffer
// the final buffer is what we print

static size_t match_sequence(typevec_t *sequence, const typevec_t *entries, size_t start, collapsed_t *collapsed)
{
    size_t seq_len = typevec_len(sequence);
    size_t ent_len = typevec_len(entries);

    // see how many times the sequence repeats
    // if it doesnt repeat then return false
    size_t count = 0;
    size_t j = 0;
    for (size_t i = start; i < ent_len; i++)
    {
        const entry_t *a = typevec_offset(entries, i);
        const entry_t *b = typevec_offset(sequence, j);

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
        return 0;

    // we have a match
    // we need to store the sequence and the repeat count
    collapsed->sequence = typevec_slice(sequence, 0, seq_len);
    collapsed->repeat = count;
    return count * seq_len;
}

static size_t collapse_frame(const typevec_t *entries, size_t start, arena_t *arena, collapsed_t *collapsed)
{
    size_t len = typevec_len(entries);
    typevec_t *buffer = typevec_new(sizeof(entry_t), len, arena);
    if (start <= len)
    {
        // push the first entry
        const entry_t *first = typevec_offset(entries, start);
        typevec_push(buffer, first);

        for (size_t i = start + 1; i < len; i++)
        {
            const entry_t *entry = typevec_offset(entries, i);
            CTASSERTF(entry != NULL, "entry at %zu is NULL", i);

            // we might have a match
            if (entry->address == first->address)
            {
                size_t count = match_sequence(buffer, entries, i, collapsed);
                if (count) return count;

                // i think this is correct...
                break;
            }
            else
            {
                // we dont have a match, so we push the entry and start again
                typevec_push(buffer, entry);
            }
        }
    }

    // if we get here then no match was found
    collapsed->entry = typevec_offset(entries, start);
    return 1;
}

static typevec_t *collapse_frames(const typevec_t *frames, arena_t *arena)
{
    size_t len = typevec_len(frames);
    typevec_t *result = typevec_new(sizeof(collapsed_t), len, arena);

    for (size_t i = 0; i < len; i++)
    {
        collapsed_t collapsed = {0};
        size_t count = collapse_frame(frames, i, arena, &collapsed);

        CTASSERTF(count > 0, "count of 0 at %zu", i);

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

static size_t get_largest_entry(const typevec_t *entries)
{
    size_t len = typevec_len(entries);
    size_t largest = 0;

    for (size_t i = 0; i < len; i++)
    {
        const entry_t *entry = typevec_offset(entries, i);
        CTASSERTF(entry != NULL, "entry at %zu is NULL", i);

        size_t width = strlen(entry->symbol);
        largest = MAX(width, largest);
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

static symbol_match_info_t get_largest_collapsed_symbol(const collapsed_t *entries, size_t len)
{
    CTASSERT(entries != NULL);

    size_t largest = strlen(kUnknownSymbol);

    for (size_t i = 0; i < len; i++)
    {
        const collapsed_t *entry = entries + i;
        CTASSERTF(entry != NULL, "entry at %zu is NULL", i);

        if (entry->entry == NULL)
        {
            symbol_match_info_t info = {
                .largest_symbol = largest,
                .count = i
            };
            return info;
        }

        size_t width = strlen(entry->entry->symbol);
        largest = MAX(width, largest);
    }

    symbol_match_info_t info = {
        .largest_symbol = largest,
        .count = len
    };
    return info;
}

static char *fmt_entry_location(backtrace_t *pass, const entry_t *entry)
{
    print_backtrace_t config = pass->options;

    frame_resolve_t resolved = entry->info;

    if (resolved & eResolveFile)
    {
        if (resolved & eResolveLine)
        {
            where_t where = {
                .first_line = entry->line,
            };

            source_config_t source_config = {
                .context = pass->format_context,
                .colour = eColourDefault,
                .heading_style = config.heading_style,
                .zero_indexed_lines = config.zero_indexed_lines,
            };

            char *out = fmt_source_location(source_config, entry->file, where);
            return str_format(pass->format_context.arena, "(%s)", out);
        }

        return entry->file;
    }

    return colour_format(pass->format_context, COLOUR_ADDR, "0x%" PRI_ADDRESS, entry->address);
}

static char *fmt_entry(backtrace_t *pass, size_t symbol_align, const entry_t *entry)
{
    print_backtrace_t config = pass->options;
    print_options_t options = config.options;

    frame_resolve_t resolved = entry->info;

    bool needs_seperator = !((resolved & eResolveFile) && (resolved & eResolveLine));
    char *where = fmt_entry_location(pass, entry);
    const char *name = (resolved & eResolveName) ? entry->symbol : kUnknownSymbol;

    char *it = fmt_right_align(options.arena, symbol_align, "%s", name);
    char *coloured = colour_text(pass->format_context, COLOUR_SYMBOL, it);

    if (needs_seperator)
    {
        return str_format(options.arena, "%s @ %s", coloured, where);
    }
    else
    {
        return str_format(options.arena, "%s %s", coloured, where);
    }
}

static char *fmt_index(backtrace_t *pass, size_t index)
{
    print_backtrace_t config = pass->options;
    print_options_t options = config.options;

    char *idx = fmt_left_align(options.arena, pass->index_align, "%zu", index);
    return colour_format(pass->format_context, COLOUR_INDEX, "[%s]", idx);
}

static void print_single_frame(backtrace_t *pass, size_t index, const entry_t *entry)
{
    print_backtrace_t options = pass->options;
    print_options_t base = options.options;
    char *idx = fmt_index(pass, index);
    io_printf(base.io, "%s %s\n", idx, fmt_entry(pass, pass->symbol_align, entry));
}

static void print_frame_sequence(backtrace_t *pass, size_t index, const typevec_t *sequence, size_t repeat)
{
    print_backtrace_t options = pass->options;
    print_options_t base = options.options;

    size_t len = typevec_len(sequence);

    size_t largest = get_largest_entry(sequence);

    char *idx = fmt_index(pass, index);
    char *coloured = colour_format(pass->format_context, COLOUR_RECURSE, "%zu", repeat);
    io_printf(base.io, "%s repeats %s times\n", idx, coloured);

    for (size_t i = 0; i < len; i++)
    {
        const entry_t *entry = typevec_offset(sequence, i);
        CTASSERTF(entry != NULL, "entry at %zu is NULL", i);

        char *it = fmt_entry(pass, largest, entry);
        char *inner = colour_format(pass->format_context, COLOUR_RECURSE, "[%zu]", i);
        io_printf(base.io, " - %s %s\n", inner, it);
    }
}

static void print_collapsed(backtrace_t *pass, size_t index, const collapsed_t *collapsed)
{
    if (collapsed->entry != NULL)
    {
        print_single_frame(pass, index, collapsed->entry);
    }
    else
    {
        print_frame_sequence(pass, index, collapsed->sequence, collapsed->repeat);
    }
}

USE_DECL
void print_backtrace(print_backtrace_t print, bt_report_t *report)
{
    CTASSERT(report != NULL);

    print_options_t options = print.options;

    typevec_t *frames = collapse_frames(report->entries, options.arena);

    collapsed_t *collapsed = typevec_data(frames);
    size_t frame_count = typevec_len(frames);

    symbol_match_info_t symbol_align = get_largest_collapsed_symbol(collapsed, frame_count);
    size_t align = get_num_width(frame_count);

    backtrace_t pass = {
        .options = print,
        .format_context = format_context_make(options),
        .frames = frames,
        .index_align = align,
        .symbol_align = symbol_align.largest_symbol,
    };

    for (size_t i = 0; i < frame_count; i++)
    {
        const collapsed_t *frame = typevec_offset(frames, i);
        CTASSERTF(frame != NULL, "collapsed at %zu is NULL", i);

        // this logic handles aligning symbol names on a per region basis
        // a region is defined as consecutive frames that were not recursive
        if (symbol_align.count > 0)
        {
            symbol_align.count -= 1;
        }
        else
        {
            if (frame->entry != NULL)
            {
                size_t remaining = frame_count - i;
                symbol_align = get_largest_collapsed_symbol(frame, remaining);
                pass.symbol_align = symbol_align.largest_symbol;
            }
        }

        print_collapsed(&pass, i, frame);
    }
}

USE_DECL
bt_report_t *bt_report_new(arena_t *arena)
{
    CTASSERT(arena != NULL);

    bt_report_t result = {
        .arena = arena,
        .entries = typevec_new(sizeof(entry_t), 4, arena),
    };

    bt_report_t *report = ARENA_MALLOC(sizeof(bt_report_t), "bt_report", NULL, arena);
    *report = result;

    ARENA_IDENTIFY(report->entries, "entries", report, arena);

    return report;
}

static void read_stacktrace_frame(void *user, const bt_frame_t *frame)
{
    bt_report_add(user, frame);
}

USE_DECL
bt_report_t *bt_report_collect(arena_t *arena)
{
    bt_report_t *report = bt_report_new(arena);

    bt_read(read_stacktrace_frame, report);

    return report;
}

USE_DECL
void bt_report_add(bt_report_t *report, const bt_frame_t *frame)
{
    CTASSERT(report != NULL);
    CTASSERT(frame != NULL);

    char path[1024];
    char name[1024];

    bt_symbol_t symbol = {
        .name = text_make(name, sizeof(name)),
        .path = text_make(path, sizeof(path)),
    };
    frame_resolve_t info = bt_resolve_symbol(frame, &symbol);

    entry_t entry = {
        .info = info,
        .file = arena_strndup(symbol.path.text, symbol.path.size, report->arena),
        .symbol = arena_strndup(symbol.name.text, symbol.name.size, report->arena),
        .line = symbol.line,
        .address = frame->address,
    };

    typevec_push(report->entries, &entry);
}
