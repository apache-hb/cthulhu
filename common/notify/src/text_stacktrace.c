#include "base/panic.h"
#include "common.h"

#include "core/macros.h"
#include "io/io.h"
#include "memory/memory.h"
#include "notify/text.h"

#include "stacktrace/stacktrace.h"

#include "std/str.h"
#include "std/typed/vector.h"

#include <ctype.h>
#include <string.h>

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
    void *address;
} bt_entry_t;

static size_t get_max_symbol_width(const bt_report_t *report)
{
    CTASSERT(report != NULL);

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

static const char *fmt_recurse(const text_colour_t *colour, const bt_entry_t *entry)
{
    if (entry->recurse == 0)
        return "";

    char *fmt = fmt_coloured(colour, eColourYellow, "%zu", entry->recurse);

    return format(" x %s", fmt);
}

// state for a backtrace format run
typedef struct bt_format_t
{
    /// symbol name right align width
    size_t symbol_align;

    /// frame index number align width
    size_t index_align;

    /// formatting config
    text_config_t config;

    /// the file cache
    /// map_t<const char*, text_cache_t*>
    cache_map_t *file_cache;

    /// current frame index
    size_t index;
} bt_format_t;

static void print_frame_index(bt_format_t *pass, const bt_entry_t *entry, size_t width)
{
    text_config_t config = pass->config;
    if (entry->recurse == 0)
    {
        char *line = fmt_align(width, "%zu", pass->index);
        char *idx = fmt_coloured(config.colours, eColourCyan, "[%s]", line);

        io_printf(config.io, "%s ", idx);

        pass->index += 1;
    }
    else
    {
        char *start = fmt_align(width, "%zu..%zu", pass->index, pass->index + entry->recurse);
        char *idx = fmt_coloured(config.colours, eColourCyan, "[%s]", start);

        io_printf(config.io, "%s ", idx);

        pass->index += entry->recurse + 1;
    }
}

// print a symbol with no extra info
static void print_frame(bt_format_t *pass, const bt_entry_t *entry)
{
    text_config_t config = pass->config;

    print_frame_index(pass, entry, pass->index_align);

    // right align the address
    char *addr = fmt_align(pass->symbol_align, "0x%p", entry->address);
    char *coloured = fmt_coloured(config.colours, eColourRed, "%s", addr);
    io_printf(config.io, "+%s%s\n", coloured, fmt_recurse(config.colours, entry));
}

// we know symbol info but not file info
static void print_simple(bt_format_t *pass, const bt_entry_t *entry)
{
    text_config_t config = pass->config;

    const char *line = (entry->info & eResolveLine)
        ? format(":%zu", get_offset_line(config.config, entry->line))
        : format(" @ %s", fmt_coloured(config.colours, eColourRed, "0x%p", entry->address));

    // right align the symbol
    size_t pad = get_symbol_padding(pass->symbol_align, entry);
    char *padding = str_repeat(" ", pad);

    char *symbol = fmt_coloured(config.colours, eColourBlue, "%s", entry->symbol);

    const char *recurse = fmt_recurse(config.colours, entry);

    print_frame_index(pass, entry, pass->index_align);

    if (entry->info & eResolveFile)
    {
        io_printf(config.io, "%s%s%s (%s%s)\n", padding, symbol, recurse, entry->file, line);
    }
    else
    {
        io_printf(config.io, "%s%s%s%s\n", padding, symbol, recurse, line);
    }
}

static text_cache_t *get_file(bt_format_t *format, const char *path)
{
    CTASSERT(format != NULL);
    CTASSERT(path != NULL);

    file_config_t config = format->config.config;
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

static bool print_single_line(text_config_t config, text_cache_t *cache, size_t data_line, size_t align, bool always_print)
{
    text_view_t text = cache_get_line(cache, data_line);
    if (!always_print && is_text_empty(&text))
        return false;

    char *line_padding = str_repeat(" ", align + 1);
    io_printf(config.io, "%s | %.*s\n", line_padding, (int)text.size, text.text);

    return true;
}

static void print_with_source(bt_format_t *pass, const bt_entry_t *entry, text_cache_t *cache)
{
    text_config_t config = pass->config;

    size_t line = get_offset_line(config.config, entry->line);
    size_t data_line = entry->line - 1;

    size_t align = get_num_width(line + 2);

    print_frame_index(pass, entry, 0);

    const char *recurse = fmt_recurse(config.colours, entry);
    const char *fn = fmt_coloured(config.colours, eColourBlue, "%s", entry->symbol);

    io_printf(config.io, "inside symbol %s%s\n", fn, recurse);

    char *header = format("%s => %s:%zu", str_repeat(" ", align - 2), entry->file, line);
    io_printf(config.io, "%s\n", header);

    // if this line is not empty, we need to print the next line unconditionally
    bool was_empty = false;
    if (data_line > 2)
    {
        was_empty = print_single_line(config, cache, data_line - 2, align, false);
    }

    // if there is 1 line before the line we are printing
    if (data_line > 1)
    {
        print_single_line(config, cache, data_line - 1, align, was_empty);
    }

    text_view_t view = cache_get_line(cache, data_line);
    char *line_num = fmt_align(align, "%zu", line);
    char *line_fmt = fmt_coloured(config.colours, eColourWhite, " %s > %.*s\n", line_num, (int)view.size, view.text);
    io_printf(config.io, "%s", line_fmt);

    size_t len = cache_count_lines(cache);
    bool print_after = (data_line + 2 < len) && !is_line_empty(cache, data_line + 2);

    if (data_line + 1 < len)
    {
        print_single_line(config, cache, data_line + 1, align, print_after);
    }

    if (data_line + 2 < len)
    {
        // the final line is always optional
        print_single_line(config, cache, data_line + 2, align, false);
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
void bt_report_finish(text_config_t config, bt_report_t *report)
{
    CTASSERT(report != NULL);

    bt_format_t pass = {
        .symbol_align = get_max_symbol_width(report),
        .index_align = get_max_index_width(report),
        .config = config,
        .file_cache = cache_map_new(report->total_frames),
        .index = 0,
    };

    // print the header
    if (config.config.print_header)
    {
        io_printf(config.io, " === backtrace (%zu frames) ===\n", report->total_frames);
    }

    size_t len = typevec_len(report->entries);
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

// the length of a pointer in hex, plus the 0x prefix
#define PTR_TEXT_LEN (2 + 2 * sizeof(void*))

static bool check_recurse(bt_report_t *report, const frame_t *frame)
{
    if (typevec_len(report->entries) == 0)
        return false;

    bt_entry_t *last = typevec_offset(report->entries, typevec_len(report->entries) - 1);
    CTASSERT(last != NULL);

    if (last->address != (void*)frame->address)
        return false;

    last->recurse++;
    report->max_consecutive_frames = MAX(report->max_consecutive_frames, last->recurse);
    return true;
}

bt_report_t bt_report_new(arena_t *arena)
{
    bt_report_t report = {
        .arena = arena,
        .entries = typevec_new(sizeof(bt_entry_t), 4, arena),
        .longest_symbol = 0,
        .max_consecutive_frames = 0,
        .total_frames = 0,
        .last_consecutive_index = 0,
    };

    return report;
}

static void read_stacktrace_frame(void *user, const frame_t *frame)
{
    bt_report_add(user, frame);
}

bt_report_t bt_report_collect(arena_t *arena)
{
    bt_report_t report = bt_report_new(arena);

    bt_read(read_stacktrace_frame, &report);

    return report;
}

USE_DECL
void bt_report_add(bt_report_t *report, const frame_t *frame)
{
    CTASSERT(report != NULL);
    CTASSERT(frame != NULL);

    report->total_frames += 1;

    if (check_recurse(report, frame))
    {
        report->last_consecutive_index = report->total_frames;
        return;
    }

    symbol_t symbol = { 0 };
    frame_resolve_t info = bt_resolve_symbol(frame, &symbol);

    bt_entry_t entry = {
        .info = info,

        .file = ctu_strdup(symbol.file, report->arena),
        .symbol = ctu_strdup(symbol.name, report->arena),
        .line = symbol.line,
        .address = (void*)frame->address,
    };

    size_t symbol_len = (info & eResolveName) ? strlen(entry.symbol) : PTR_TEXT_LEN;
    report->longest_symbol = MAX(report->longest_symbol, symbol_len);

    typevec_push(report->entries, &entry);
}
