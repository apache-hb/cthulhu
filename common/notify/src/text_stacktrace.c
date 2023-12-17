#include "base/panic.h"
#include "common.h"

#include "core/macros.h"
#include "io/io.h"
#include "memory/memory.h"
#include "notify/text.h"

#include "stacktrace/stacktrace.h"

#include "std/str.h"
#include "std/typed/vector.h"
#include <string.h>

typedef struct bt_entry_t
{
    frame_resolve_t info;

    // the number of times this frame has recursed in sequence
    size_t recurse;

    char *file;
    char *symbol;
    size_t line;
    void *address;
} bt_entry_t;

typedef struct bt_report_t
{
    typevec_t *entries;

    /// @brief the longest symbol name
    size_t longest_symbol;

    /// @brief the largest number of consecutive frames
    size_t max_consecutive_frames;

    /// @brief the total number of frames consumed
    /// this is not the same as the number of entries as it includes recursed frames
    size_t total_frames;

    /// @brief the index of the last consecutive frame
    size_t last_consecutive_index;
} bt_report_t;

static size_t get_max_symbol_width(const bt_report_t *report)
{
    CTASSERT(report != NULL);

    if (report->max_consecutive_frames == 0)
    {
        return report->longest_symbol;
    }

    // +3 for the ` x ` suffix
    return report->longest_symbol + 3 + get_num_width(report->max_consecutive_frames);
}

static size_t get_max_line_width(const bt_report_t *report)
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
    size_t pad = align - strlen(entry->symbol);

    if (entry->recurse == 0)
        return pad;

    pad -= 3 + get_num_width(entry->recurse);

    return pad;
}

static const char *fmt_recurse(const bt_entry_t *entry)
{
    if (entry->recurse == 0)
        return "";

    char *fmt = fmt_coloured(kDefaultColour, eColourYellow, "%zu", entry->recurse);

    return format(" x %s", fmt);
}

static void print_frame(text_config_t config, size_t align, const bt_entry_t *entry)
{
    // right align the address
    char *addr = fmt_align(align, "0x%p", entry->address);
    char *coloured = fmt_coloured(config.colours, eColourRed, "%s", addr);
    io_printf(config.io, "+%s%s\n", coloured, fmt_recurse(entry));
}

static void print_simple(text_config_t config, size_t align, const bt_entry_t *entry)
{
    const char *line = (entry->info & eResolveLine)
        ? format(":%zu", get_offset_line(config.config, entry->line))
        : format(" @ %s", fmt_coloured(config.colours, eColourRed, "0x%p", entry->address));

    // right align the symbol
    size_t pad = get_symbol_padding(align, entry);
    char *padding = str_repeat(" ", pad);

    char *symbol = fmt_coloured(config.colours, eColourBlue, "%s", entry->symbol);

    const char *recurse = fmt_recurse(entry);

    if (entry->info & eResolveFile)
    {
        io_printf(config.io, "%s%s%s (%s%s)\n", padding, symbol, recurse, entry->file, line);
    }
    else
    {
        io_printf(config.io, "%s%s%s%s\n", padding, symbol, recurse, line);
    }
}

static void print_source(text_config_t config, size_t align, const bt_entry_t *entry)
{
    if (entry->info & eResolveFile)
    {
        io_t *io = io_file(entry->file, eAccessRead | eAccessText);
        if (io_error(io) != 0)
        {
            print_simple(config, align, entry);
        }
        else
        {
            // TODO: print actual source text
            print_simple(config, align, entry);
        }

        io_close(io);
    }
    else
    {
        print_simple(config, align, entry);
    }
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

bt_report_t *bt_report_new(void)
{
    bt_report_t *report = MEM_ALLOC(sizeof(bt_report_t), "bt_report", NULL);

    report->entries = typevec_new(sizeof(bt_entry_t), 4);
    report->longest_symbol = 0;
    report->max_consecutive_frames = 0;
    report->total_frames = 0;
    report->last_consecutive_index = 0;

    return report;
}

static void read_stacktrace_frame(void *user, const frame_t *frame)
{
    bt_report_add(user, frame);
}

bt_report_t *bt_report_collect(void)
{
    bt_report_t *report = bt_report_new();

    stacktrace_read(read_stacktrace_frame, report);

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
    frame_resolve_t info = frame_resolve(frame, &symbol);

    bt_entry_t entry = {
        .info = info,

        .file = ctu_strdup(symbol.file),
        .symbol = ctu_strdup(symbol.name),
        .line = symbol.line,
        .address = (void*)frame->address,
    };

    size_t symbol_len = (info & eResolveName) ? strlen(entry.symbol) : PTR_TEXT_LEN;
    report->longest_symbol = MAX(report->longest_symbol, symbol_len);

    typevec_push(report->entries, &entry);
}

USE_DECL
void bt_report_finish(text_config_t config, bt_report_t *report)
{
    CTASSERT(report != NULL);

    size_t width = get_max_line_width(report);

    size_t symbol_align = get_max_symbol_width(report);

    size_t index = 0;
    size_t len = typevec_len(report->entries);
    for (size_t i = 0; i < len; i++)
    {
        bt_entry_t *entry = typevec_offset(report->entries, i);
        CTASSERT(entry != NULL);

        if (entry->recurse == 0)
        {
            char *line = fmt_align(width, "%zu", index);
            char *idx = fmt_coloured(config.colours, eColourCyan, "[%s]", line);

            io_printf(config.io, "%s ", idx);

            index += 1;
        }
        else
        {
            char *start = fmt_align(width, "%zu..%zu", index, index + entry->recurse);
            char *idx = fmt_coloured(config.colours, eColourCyan, "[%s]", start);

            io_printf(config.io, "%s ", idx);

            index += entry->recurse + 1;
        }

        if (entry->info == eResolveNothing)
        {
            print_frame(config, symbol_align, entry);
        }
        else
        {
            print_source(config, symbol_align, entry);
        }
    }
}
