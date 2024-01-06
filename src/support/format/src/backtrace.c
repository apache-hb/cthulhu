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

typedef struct bt_report_t
{
    /// @brief memory pool
    arena_t *arena;

    /// @brief all entries
    typevec_t *entries;
} bt_report_t;

/// @brief a single backtrace entry
typedef struct bt_entry_t
{
    /// @brief what symbol info has been resolved by the backend?
    frame_resolve_t info;

    /// @brief recursion count for this symbol
    /// the number of times this frame has recursed in sequence
    size_t recurse;

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
} bt_entry_t;

// state for a backtrace format run
typedef struct bt_format_t
{
    print_backtrace_t options;

    format_context_t format_context;

    /// symbol name right align width
    size_t symbol_align;

    /// frame index number align width
    size_t index_align;

    /// the file cache
    /// map_t<const char*, text_cache_t*>
    cache_map_t *file_cache;

    /// current frame index
    size_t index;
} bt_format_t;

typedef struct search_t
{
    arena_t *arena;
    size_t len;
    typevec_t *longest;
} search_t;

typedef struct suffix_tree_t
{
    bt_address_t address;
    map_t *children;
    typevec_t *indices;
} suffix_tree_t;

static suffix_tree_t *suffix_tree_new(bt_address_t address, arena_t *arena)
{
    suffix_tree_t *tree = ARENA_MALLOC(arena, sizeof(suffix_tree_t), "suffix_tree", NULL);
    tree->address = address;
    tree->children = map_new_arena(4, arena);
    tree->indices = typevec_new(sizeof(size_t), 4, arena);

    return tree;
}

static void insert_suffix(search_t *longest, suffix_tree_t *tree, typevec_t *slice, size_t index, typevec_t *original, size_t level)
{
    if (tree->indices == NULL)
    {
        tree->indices = typevec_new(sizeof(size_t), 4, longest->arena);
    }

    typevec_push(tree->indices, &index);
    size_t slice_len = typevec_len(slice);
    size_t longest_len = typevec_len(longest->longest);
    if (slice_len > 1 && longest_len < level)
    {
        longest->len = level;
        longest->longest = typevec_slice(original, 0, level);
    }

    if (slice_len == 0)
        return;
}

static typevec_t *longest_repeated_sequence(typevec_t *symbols, arena_t *arena)
{
    search_t search = {
        .arena = arena,
        .longest = typevec_new(sizeof(size_t), 4, arena),
    };
    suffix_tree_t *root = suffix_tree_new(0, arena);
    size_t len = typevec_len(symbols);
    for (size_t i = 0; i < len; i++)
    {
        typevec_t *slice = typevec_slice(symbols, i, len);
        insert_suffix(&search, root, slice, i, symbols, 0);
    }

    return NULL;
}

static size_t get_max_symbol_width(const bt_report_t *report)
{
    CTASSERT(report != NULL);

    size_t len = typevec_len(report->entries);
    size_t largest = 0;
    for (size_t i = 0; i < len; i++)
    {
        const bt_entry_t *entry = typevec_offset(report->entries, i);
        CTASSERT(entry != NULL);

        size_t width = strlen(entry->symbol);
        if (width > largest)
            largest = width;
    }

    if (report->max_consecutive_frames == 0)
        return report->longest_symbol;

    // +3 for the ` x ` suffix
    return report->longest_symbol + 3 + get_num_width(report->max_consecutive_frames);
}

static size_t get_max_index_width(const bt_report_t *report)
{
    CTASSERT(report != NULL);

    size_t width = get_num_width(report->total_frames);

    if (report->max_consecutive_frames == 0)
        return width;

    size_t span = get_num_width(report->last_consecutive_index - report->max_consecutive_frames) + 2 + get_num_width(report->last_consecutive_index);

    return span;
}

static size_t get_symbol_padding(size_t align, const bt_entry_t *entry)
{
    if (align == 0) return 0;

    size_t pad = align - strlen(entry->symbol);

    if (entry->recurse == 0)
        return pad;

    pad -= 3 + get_num_width(entry->recurse);

    return pad;
}

static const char *fmt_recurse(bt_format_t *pass, const bt_entry_t *entry)
{
    if (entry->recurse == 0)
        return "";

    char *fmt = colour_format(pass->format_context, COLOUR_RECURSE, "%zu", entry->recurse);

    return format(" x %s", fmt);
}

static void print_frame_index(bt_format_t *pass, const bt_entry_t *entry, size_t width)
{
    print_backtrace_t config = pass->options;
    print_options_t options = config.options;

    if (entry->recurse == 0)
    {
        char *line = fmt_align(options.arena, width, "%zu", pass->index);
        char *idx = colour_format(pass->format_context, COLOUR_INDEX, "[%s]", line);

        io_printf(options.io, "%s ", idx);

        pass->index += 1;
    }
    else
    {
        char *start = fmt_align(options.arena, width, "%zu..%zu", pass->index, pass->index + entry->recurse);
        char *idx = colour_format(pass->format_context, COLOUR_INDEX, "[%s]", start);

        io_printf(options.io, "%s ", idx);

        pass->index += entry->recurse + 1;
    }
}

// print a symbol with no extra info
static void print_frame(bt_format_t *pass, const bt_entry_t *entry)
{
    print_backtrace_t config = pass->options;
    print_options_t options = config.options;

    print_frame_index(pass, entry, pass->index_align);

    // right align the address
    char *addr = fmt_align(options.arena, pass->symbol_align, "0x%p", entry->address);
    char *coloured = colour_text(pass->format_context, COLOUR_ADDR, addr);
    io_printf(options.io, "+%s%s\n", coloured, fmt_recurse(pass, entry));
}

// we know symbol info but not file info
static void print_simple(bt_format_t *pass, const bt_entry_t *entry)
{
    print_backtrace_t config = pass->options;
    print_options_t options = config.options;

    const char *line = (entry->info & eResolveLine)
        ? format(":%zu", get_offset_line(config.zero_indexed_lines, entry->line))
        : format(" @ %s", colour_format(pass->format_context, COLOUR_ADDR, "0x%p", entry->address));

    // right align the symbol
    size_t pad = get_symbol_padding(pass->symbol_align, entry);
    char *padding = str_repeat(" ", pad);

    char *symbol = colour_text(pass->format_context, COLOUR_SYMBOL, entry->symbol);

    const char *recurse = fmt_recurse(pass, entry);

    print_frame_index(pass, entry, pass->index_align);

    if (entry->info & eResolveFile)
    {
        io_printf(options.io, "%s%s%s (%s%s)\n", padding, symbol, recurse, entry->file, line);
    }
    else
    {
        io_printf(options.io, "%s%s%s%s\n", padding, symbol, recurse, line);
    }
}

static text_cache_t *get_file(bt_format_t *format, const char *path)
{
    CTASSERT(format != NULL);
    CTASSERT(path != NULL);

    print_backtrace_t config = format->options;
    if (!config.print_source) return NULL;

    return cache_emplace_file(format->file_cache, path);
}

static bool is_text_empty(const text_view_t *view)
{
    CTASSERT(view != NULL);

    for (size_t i = 0; i < view->size; i++)
        if (!isspace(view->text[i]))
            return false;

    return true;
}

static bool is_line_empty(text_cache_t *cache, size_t line)
{
    CTASSERT(cache != NULL);

    text_view_t view = cache_get_line(cache, line);
    return is_text_empty(&view);
}

static bool print_single_line(io_t *io, text_cache_t *cache, size_t data_line, size_t align, bool always_print)
{
    text_view_t text = cache_get_line(cache, data_line);
    if (!always_print && is_text_empty(&text))
        return false;

    char *line_padding = str_repeat(" ", align + 1);
    io_printf(io, "%s | %.*s\n", line_padding, (int)text.size, text.text);

    return true;
}

static void print_with_source(bt_format_t *pass, const bt_entry_t *entry, text_cache_t *cache)
{
    print_backtrace_t config = pass->options;
    print_options_t options = config.options;

    size_t line = get_offset_line(config.zero_indexed_lines, entry->line);
    size_t data_line = entry->line - 1;

    size_t align = get_num_width(line + 2);

    print_frame_index(pass, entry, 0);

    const char *recurse = fmt_recurse(pass, entry);
    const char *fn = colour_text(pass->format_context, COLOUR_SYMBOL, entry->symbol);

    io_printf(options.io, "inside symbol %s%s\n", fn, recurse);

    source_config_t source_config = {
        .context = pass->format_context,
        .heading_style = config.heading_style,
        .zero_indexed_lines = config.zero_indexed_lines,
    };

    where_t where = {
        .first_line = line,
        .first_column = 0,
    };

    char *arrow = format("%s =>", str_repeat(" ", align - 2));
    char *header = fmt_source_location(source_config, entry->file, where);
    io_printf(options.io, "%s %s\n", arrow, header);

    // if this line is not empty, we need to print the next line unconditionally
    bool was_empty = false;
    if (data_line > 2)
    {
        was_empty = print_single_line(options.io, cache, data_line - 2, align, false);
    }

    // if there is 1 line before the line we are printing
    if (data_line > 1)
    {
        print_single_line(options.io, cache, data_line - 1, align, was_empty);
    }

    text_view_t view = cache_get_line(cache, data_line);
    char *line_num = fmt_align(options.arena, align, "%zu", line);
    char *line_fmt = colour_format(pass->format_context, COLOUR_LINE, " %s > %.*s\n", line_num, (int)view.size, view.text);
    io_printf(options.io, "%s", line_fmt);

    size_t len = cache_count_lines(cache);
    bool print_after = (data_line + 2 < len) && !is_line_empty(cache, data_line + 2);

    if (data_line + 1 < len)
    {
        print_single_line(options.io, cache, data_line + 1, align, print_after);
    }

    if (data_line + 2 < len)
    {
        // the final line is always optional
        print_single_line(options.io, cache, data_line + 2, align, false);
    }
}

// we may have source information
static void print_source(bt_format_t *pass, const bt_entry_t *entry)
{
    // we found the file for this entry
    text_cache_t *cache = (entry->info & eResolveFile) ? get_file(pass, entry->file) : NULL;
    if (cache == NULL)
    {
        // we dont have the file on disk
        print_simple(pass, entry);
    }
    else
    {
        // we have the file on disk
        print_with_source(pass, entry, cache);
    }
}

USE_DECL
void print_backtrace(print_backtrace_t config, bt_report_t *report)
{
    CTASSERT(report != NULL);

    print_options_t options = config.options;

    size_t len = typevec_len(report->entries);

    bt_format_t pass = {
        .options = config,
        .format_context = format_context_make(options),
        .symbol_align = get_max_symbol_width(report),
        .index_align = get_max_index_width(report),
        .file_cache = cache_map_new(len),
        .index = 0,
    };

    // print the header
    if (config.header_message)
    {
        io_printf(options.io, " === backtrace (%zu frames) ===\n", len);

        if (config.header_message != NULL)
            io_printf(options.io, "%s\n", config.header_message);
    }

    for (size_t i = 0; i < len; i++)
    {
        bt_entry_t *entry = typevec_offset(report->entries, i);
        CTASSERT(entry != NULL);

        if (entry->info == eResolveNothing)
        {
            print_frame(&pass, entry);
        }
        else
        {
            print_source(&pass, entry);
        }
    }

    // close all the files
    cache_map_delete(pass.file_cache);
}

bt_report_t *bt_report_new(arena_t *arena)
{
    bt_report_t result = {
        .arena = arena,
        .entries = typevec_new(sizeof(bt_entry_t), 4, arena),
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

    bt_entry_t entry = {
        .info = info,
        .file = arena_strndup(symbol.path.text, symbol.path.size, report->arena),
        .symbol = arena_strndup(symbol.name.text, symbol.name.size, report->arena),
        .line = symbol.line,
        .address = frame->address,
    };

    typevec_push(report->entries, &entry);
}
