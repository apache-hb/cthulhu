#include "base/panic.h"
#include "common.h"

#include "core/macros.h"
#include "io/io.h"
#include "memory/arena.h"
#include "format/notify.h"

#include "backtrace/backtrace.h"
#include "format/backtrace.h"

#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"

#include <ctype.h>
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
    print_backtrace_t options;

    /// format context
    format_context_t format_context;

    /// the collapsed frames
    typevec_t *frames;

    size_t index_align;
    size_t symbol_align;

    /// the file cache
    /// map_t<const char*, text_cache_t*>
    cache_map_t *file_cache;
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

// static void print_frame_index(backtrace_t *pass, const bt_entry_t *entry, size_t width)
// {
//     print_backtrace_t config = pass->options;
//     print_options_t options = config.options;

//     if (entry->recurse == 0)
//     {
//         char *line = fmt_left_align(options.arena, width, "%zu", pass->index);
//         char *idx = colour_format(pass->format_context, COLOUR_INDEX, "[%s]", line);

//         io_printf(options.io, "%s ", idx);

//         pass->index += 1;
//     }
//     else
//     {
//         char *start = fmt_left_align(options.arena, width, "%zu..%zu", pass->index, pass->index + entry->recurse);
//         char *idx = colour_format(pass->format_context, COLOUR_INDEX, "[%s]", start);

//         io_printf(options.io, "%s ", idx);

//         pass->index += entry->recurse + 1;
//     }
// }

// // print a symbol with no extra info
// static void print_frame(backtrace_t *pass, const bt_entry_t *entry)
// {
//     print_backtrace_t config = pass->options;
//     print_options_t options = config.options;

//     print_frame_index(pass, entry, pass->index_align);

//     // right align the address
//     char *addr = fmt_left_align(options.arena, pass->symbol_align, "0x%p", entry->address);
//     char *coloured = colour_text(pass->format_context, COLOUR_ADDR, addr);
//     io_printf(options.io, "+%s%s\n", coloured, fmt_recurse(pass, entry));
// }

// // we know symbol info but not file info
// static void print_simple(backtrace_t *pass, const bt_entry_t *entry)
// {
//     print_backtrace_t config = pass->options;
//     print_options_t options = config.options;

//     const char *line = (entry->info & eResolveLine)
//         ? format(":%zu", get_offset_line(config.zero_indexed_lines, entry->line))
//         : format(" @ %s", colour_format(pass->format_context, COLOUR_ADDR, "0x%p", entry->address));

//     // right align the symbol
//     size_t pad = get_symbol_padding(pass->symbol_align, entry);
//     char *padding = str_repeat(" ", pad);

//     char *symbol = colour_text(pass->format_context, COLOUR_SYMBOL, entry->symbol);

//     const char *recurse = fmt_recurse(pass, entry);

//     print_frame_index(pass, entry, pass->index_align);

//     if (entry->info & eResolveFile)
//     {
//         io_printf(options.io, "%s%s%s (%s%s)\n", padding, symbol, recurse, entry->file, line);
//     }
//     else
//     {
//         io_printf(options.io, "%s%s%s%s\n", padding, symbol, recurse, line);
//     }
// }

// static text_cache_t *get_file(backtrace_t *format, const char *path)
// {
//     CTASSERT(format != NULL);
//     CTASSERT(path != NULL);

//     print_backtrace_t config = format->options;
//     if (!config.print_source) return NULL;

//     return cache_emplace_file(format->file_cache, path);
// }

// static bool is_text_empty(const text_view_t *view)
// {
//     CTASSERT(view != NULL);

//     for (size_t i = 0; i < view->size; i++)
//         if (!isspace(view->text[i]))
//             return false;

//     return true;
// }

// static bool is_line_empty(text_cache_t *cache, size_t line)
// {
//     CTASSERT(cache != NULL);

//     text_view_t view = cache_get_line(cache, line);
//     return is_text_empty(&view);
// }

// static bool print_single_line(io_t *io, text_cache_t *cache, size_t data_line, size_t align, bool always_print)
// {
//     text_view_t text = cache_get_line(cache, data_line);
//     if (!always_print && is_text_empty(&text))
//         return false;

//     char *line_padding = str_repeat(" ", align + 1);
//     io_printf(io, "%s | %.*s\n", line_padding, (int)text.size, text.text);

//     return true;
// }

// static void print_with_source(backtrace_t *pass, const bt_entry_t *entry, text_cache_t *cache)
// {
//     print_backtrace_t config = pass->options;
//     print_options_t options = config.options;

//     size_t line = get_offset_line(config.zero_indexed_lines, entry->line);
//     size_t data_line = entry->line - 1;

//     size_t align = get_num_width(line + 2);

//     print_frame_index(pass, entry, 0);

//     const char *recurse = fmt_recurse(pass, entry);
//     const char *fn = colour_text(pass->format_context, COLOUR_SYMBOL, entry->symbol);

//     io_printf(options.io, "inside symbol %s%s\n", fn, recurse);

//     source_config_t source_config = {
//         .context = pass->format_context,
//         .heading_style = config.heading_style,
//         .zero_indexed_lines = config.zero_indexed_lines,
//     };

//     where_t where = {
//         .first_line = line,
//         .first_column = 0,
//     };

//     char *arrow = format("%s =>", str_repeat(" ", align - 2));
//     char *header = fmt_source_location(source_config, entry->file, where);
//     io_printf(options.io, "%s %s\n", arrow, header);

//     // if this line is not empty, we need to print the next line unconditionally
//     bool was_empty = false;
//     if (data_line > 2)
//     {
//         was_empty = print_single_line(options.io, cache, data_line - 2, align, false);
//     }

//     // if there is 1 line before the line we are printing
//     if (data_line > 1)
//     {
//         print_single_line(options.io, cache, data_line - 1, align, was_empty);
//     }

//     text_view_t view = cache_get_line(cache, data_line);
//     char *line_num = fmt_left_align(options.arena, align, "%zu", line);
//     char *line_fmt = colour_format(pass->format_context, COLOUR_LINE, " %s > %.*s\n", line_num, (int)view.size, view.text);
//     io_printf(options.io, "%s", line_fmt);

//     size_t len = cache_count_lines(cache);
//     bool print_after = (data_line + 2 < len) && !is_line_empty(cache, data_line + 2);

//     if (data_line + 1 < len)
//     {
//         print_single_line(options.io, cache, data_line + 1, align, print_after);
//     }

//     if (data_line + 2 < len)
//     {
//         // the final line is always optional
//         print_single_line(options.io, cache, data_line + 2, align, false);
//     }
// }

// // we may have source information
// static void print_source(backtrace_t *pass, const bt_entry_t *entry)
// {
//     // we found the file for this entry
//     text_cache_t *cache = (entry->info & eResolveFile) ? get_file(pass, entry->file) : NULL;
//     if (cache == NULL)
//     {
//         // we dont have the file on disk
//         print_simple(pass, entry);
//     }
//     else
//     {
//         // we have the file on disk
//         print_with_source(pass, entry, cache);
//     }
// }

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

static size_t get_largest_collapsed_symbol(const typevec_t *entries)
{
    size_t len = typevec_len(entries);
    size_t largest = strlen(kUnknownSymbol);

    for (size_t i = 0; i < len; i++)
    {
        const collapsed_t *entry = typevec_offset(entries, i);
        CTASSERTF(entry != NULL, "entry at %zu is NULL", i);

        if (entry->entry == NULL)
            continue;

        size_t width = strlen(entry->entry->symbol);
        largest = MAX(width, largest);
    }

    return largest;
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
void print_backtrace(print_backtrace_t config, bt_report_t *report)
{
    CTASSERT(report != NULL);

    print_options_t options = config.options;

    size_t len = typevec_len(report->entries);
    typevec_t *frames = collapse_frames(report->entries, options.arena);

    size_t symbol_align = get_largest_collapsed_symbol(frames);
    size_t frame_count = typevec_len(frames);
    size_t align = get_num_width(frame_count);

    backtrace_t pass = {
        .options = config,
        .format_context = format_context_make(options),
        .frames = frames,
        .index_align = align,
        .symbol_align = symbol_align,
        .file_cache = cache_map_new(len),
    };

    for (size_t i = 0; i < frame_count; i++)
    {
        const collapsed_t *collapsed = typevec_offset(frames, i);
        CTASSERTF(collapsed != NULL, "collapsed at %zu is NULL", i);

        print_collapsed(&pass, i, collapsed);
    }

    // close all the files
    cache_map_delete(pass.file_cache);
}

bt_report_t *bt_report_new(arena_t *arena)
{
    bt_report_t result = {
        .arena = arena,
        .entries = typevec_new(sizeof(entry_t), 4, arena),
    };

    bt_report_t *report = ARENA_MALLOC(arena, sizeof(bt_report_t), "bt_report", NULL);
    *report = result;

    ARENA_IDENTIFY(arena, report->entries, "entries", report);

    return report;
}

static void read_stacktrace_frame(void *user, const frame_t *frame)
{
    bt_report_add(user, frame);
}

bt_report_t *bt_report_collect(arena_t *arena)
{
    bt_report_t *report = bt_report_new(arena);

    bt_read(read_stacktrace_frame, report);

    return report;
}

// the length of a pointer in hex, plus the 0x prefix
#define PTR_TEXT_LEN (2 + 2 * sizeof(void*))

USE_DECL
void bt_report_add(bt_report_t *report, const frame_t *frame)
{
    CTASSERT(report != NULL);
    CTASSERT(frame != NULL);

    char path[1024];
    char name[1024];

    symbol_t symbol = {
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
