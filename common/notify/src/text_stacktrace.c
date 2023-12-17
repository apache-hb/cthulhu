#include "common.h"

#include "io/io.h"
#include "notify/text.h"

#include "stacktrace/stacktrace.h"

#include "std/str.h"

static void print_frame(text_config_t config, size_t index, const frame_t *frame)
{
    char *idx = fmt_coloured(config.colours, eColourCyan, "[%zu]", index);
    io_printf(config.io, "%s @0x%p\n", idx, (void*)frame->address);
}

void print_simple(text_config_t config, size_t index, frame_resolve_t info, const frame_t *frame, const symbol_t *symbol)
{
    char *idx = fmt_coloured(config.colours, eColourCyan, "[%zu]", index);

    const char *line = (info & eResolveLine)
        ? format(":%zu", symbol->line)
        : fmt_coloured(config.colours, eColourBlue, " @ 0x%p", (void*)frame->address);

    if (info & eResolveFile)
    {
        io_printf(config.io, "%s %s (%s%s)\n", idx, symbol->name, symbol->file, line);
    }
    else
    {
        io_printf(config.io, "%s %s%s\n", idx, symbol->name, line);
    }
}

void print_source(text_config_t config, size_t index, frame_resolve_t info, const frame_t *frame, const symbol_t *symbol)
{
    io_t *io = io_file(symbol->file, eAccessRead | eAccessText);
    if (io_error(io) != 0)
    {
        print_simple(config, index, info, frame, symbol);
    }
    else
    {
        // TODO: print actual source text
        print_simple(config, index, info, frame, symbol);
    }

    io_close(io);
}

void text_report_stacktrace(text_config_t config, size_t index, const frame_t *frame)
{
    symbol_t symbol = { 0 };
    frame_resolve_t info = frame_resolve(frame, &symbol);

    if (info == eResolveNothing)
    {
        print_frame(config, index, frame);
    }
    else
    {
        if (info & eResolveFile)
        {
            print_source(config, index, info, frame, &symbol);
        }
        else
        {
            print_simple(config, index, info, frame, &symbol);
        }
    }
}
