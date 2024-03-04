// SPDX-License-Identifier: LGPL-3.0-only

#include "common.h"

#include "base/util.h"
#include "base/panic.h"

#include "arena/arena.h"

#include "std/str.h"

#include <string.h>

size_t get_offset_line(bool zero_indexed_lines, size_t line)
{
    // if the first line is 0, then we don't need to do anything
    if (zero_indexed_lines) return line;

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

char *fmt_left_align(arena_t *arena, size_t width, const char *fmt, ...)
{
    CTASSERTF(width >= 1, "width must be at least 1 (%zu given)", width);

    va_list args;
    va_start(args, fmt);
    text_t msg = text_vformat(arena, fmt, args);
    va_end(args);

    if (msg.length >= width) return msg.text;

    size_t size = width - 1;
    char *result = ARENA_MALLOC(size, "fmt_left_align", NULL, arena);
    ctu_memset(result, ' ', width);
    ctu_memcpy(result, msg.text, msg.length);

    result[width] = '\0';

    arena_free(msg.text, msg.length, arena);

    return result;
}

char *fmt_right_align(arena_t *arena, size_t width, const char *fmt, ...)
{
    CTASSERTF(width >= 1, "width must be at least 1 (%zu given)", width);

    va_list args;
    va_start(args, fmt);
    text_t msg = text_vformat(arena, fmt, args);
    va_end(args);

    if (msg.length >= width) return msg.text;

    size_t size = width - 1;
    char *result = ARENA_MALLOC(size, "fmt_right_align", NULL, arena);
    ctu_memset(result, ' ', width);
    ctu_memcpy(result + (width - msg.length), msg.text, msg.length);

    result[width] = '\0';

    arena_free(msg.text, msg.length, arena);

    return result;
}

#define SCAN_BUILTIN_NAME "<builtin>"

static const char *const kFormatBuiltinHeading[eHeadingCount] = {
    [eHeadingGeneric] = SCAN_BUILTIN_NAME ":%" PRI_LINE "",
    [eHeadingMicrosoft] = SCAN_BUILTIN_NAME "(%" PRI_LINE ")",
};

static line_t calc_line_number(bool zero_indexed_lines, line_t line)
{
    if (zero_indexed_lines) return line;

    return line + 1;
}

static char *fmt_any_location(source_config_t config, const char *path, line_t line, column_t column)
{
    if (path == NULL)
    {
        const char *fmt = kFormatBuiltinHeading[config.heading_style];
        return colour_format(config.context, config.colour, fmt, line);
    }

    // we branch here because msvc doesnt report column numbers, only lines
    if (config.heading_style == eHeadingGeneric)
    {
        return colour_format(config.context, config.colour, "%s:%" PRI_LINE ":%" PRI_COLUMN "",
                                path, line, column);
    }
    else
    {
        return colour_format(config.context, config.colour, "%s(%" PRI_LINE ")", path, line);
    }
}

format_context_t format_context_make(print_options_t options)
{
    format_context_t context = {
        .arena = options.arena,
        .pallete = options.pallete,
    };

    return context;
}

char *fmt_source_location(source_config_t config, const char *path, where_t where)
{
    line_t first_line = calc_line_number(config.zero_indexed_lines, where.first_line);

    return fmt_any_location(config, path, first_line, where.first_column);
}
