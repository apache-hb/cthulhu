// SPDX-License-Identifier: LGPL-3.0-only

#include "format/config.h"
#include "common.h"

#include "config/config.h"

#include "io/io.h"

#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

#include "base/panic.h"
#include "base/util.h"

#include "core/macros.h"

#include <limits.h>
#include <stdint.h>

#define COLOUR_ARG eColourWhite

typedef struct format_config_t
{
    arena_t *arena;
    io_t *io;
    format_context_t context;
} format_config_t;

typedef struct alignment_info_t
{
    size_t arg_alignment;
    size_t brief_alignment;
} alignment_info_t;

// get the longest single line in a string
static size_t longest_line(const char *str)
{
    size_t longest = 0;
    size_t current = 0;

    for (size_t i = 0; str[i]; i++)
    {
        current += 1;

        if (str[i] == '\n')
        {
            longest = CT_MAX(longest, current);
            current = 0;
        }
    }

    return CT_MAX(longest, current);
}

static const char *get_arg_prefix(bool win_style, arg_style_t style)
{
    if (win_style)
        return "/";

    return cfg_arg_prefix(style);
}

static bool should_skip_arg(bool win_style, arg_style_t style)
{
    return (!win_style && style == eArgDOS);
}

static size_t get_arg_length(const cfg_info_t *info, bool win_style)
{
    size_t len = 0;
    cfg_arg_array_t args = info->args;
    for (size_t i = 0; i < args.count; i++)
    {
        cfg_arg_t arg = args.args[i];
        if (should_skip_arg(false, arg.style))
            continue;

        len += ctu_strlen(args.args[i].arg) + 1 + ctu_strlen(get_arg_prefix(win_style, arg.style));
    }

    return len;
}

static alignment_info_t get_group_alignment(const cfg_group_t *config, bool win_style)
{
    size_t longest_brief = 0;

    size_t largest = 0;
    vector_t *fields = cfg_get_fields(config);
    size_t field_count = vector_len(fields);
    for (size_t i = 0; i < field_count; i++)
    {
        const cfg_field_t *field = vector_get(fields, i);
        const cfg_info_t *info = cfg_get_info(field);
        size_t len = get_arg_length(info, win_style);
        largest = CT_MAX(largest, len);

        size_t brief_len = longest_line(info->brief);

        longest_brief = CT_MAX(longest_brief, brief_len);
    }

    alignment_info_t alignment = {
        .arg_alignment = largest,
        .brief_alignment = longest_brief + 1
    };

    return alignment;
}

// print the args for a single field
// returns the number of characters printed
static size_t print_field_args(format_config_t config, const cfg_info_t *info, bool win_style)
{
    size_t len = 0;

    cfg_arg_array_t args = info->args;
    for (size_t i = 0; i < args.count; i++)
    {
        cfg_arg_t arg = args.args[i];
        if (should_skip_arg(win_style, arg.style))
            continue;

        const char *prefix = get_arg_prefix(win_style, arg.style);
        char *coloured = colour_format(config.context, COLOUR_ARG, "%s%s", prefix, arg.arg);
        io_printf(config.io, "%s ", coloured);
        len += ctu_strlen(arg.arg) + 1 + ctu_strlen(prefix);
    }

    return len;
}

static void print_range(io_t *io, int min, int max)
{
    if (min != INT_MIN)
    {
        io_printf(io, ", min: %d", min);

        if (max != INT_MAX)
        {
            io_printf(io, ",");
        }
    }

    if (max != INT_MAX)
    {
        if (min == INT_MIN)
        {
            io_printf(io, ",");
        }

        io_printf(io, " max: %d", max);
    }
}

static const char *get_enum_option(const cfg_choice_t *choices, size_t len, size_t choice)
{
    for (size_t i = 0; i < len; i++)
    {
        if (choices[i].value == choice)
        {
            return choices[i].text;
        }
    }

    return "<unknown>";
}

static void print_enum_default(format_config_t options, const cfg_field_t *field)
{
    const cfg_enum_t *info = cfg_enum_info(field);
    const char *option = get_enum_option(info->options, info->count, info->initial);
    io_printf(options.io, "(default: %s)\n", option);
}

static void print_enum(format_config_t options, alignment_info_t alignment, const cfg_field_t *field)
{
    const cfg_enum_t *info = cfg_enum_info(field);

    char *padding = str_repeat(" ", alignment.arg_alignment, options.arena);
    io_printf(options.io, "%soptions: ", padding);

    for (size_t i = 0; i < info->count; i++)
    {
        const cfg_choice_t *choice = info->options + i;
        io_printf(options.io, "%s", choice->text);

        if (i != info->count - 1)
        {
            io_printf(options.io, ", ");
        }
    }

    io_printf(options.io, "\n");
}

static void print_flags_default(format_config_t options, const cfg_field_t *field)
{
    const cfg_enum_t *info = cfg_flags_info(field);
    io_printf(options.io, "flags (default: ");

    bool first = true;
    for (size_t i = 0; i < info->count; i++)
    {
        const cfg_choice_t choice = info->options[i];
        if (info->initial & choice.value)
        {
            if (!first)
            {
                io_printf(options.io, "|");
            }

            io_printf(options.io, "%s", choice.text);
            first = false;
        }
    }

    io_printf(options.io, ")\n");
}

static void print_flags(format_config_t options, alignment_info_t alignment, const cfg_field_t *field)
{
    const cfg_enum_t *info = cfg_flags_info(field);

    char *padding = str_repeat(" ", alignment.arg_alignment, options.arena);
    io_printf(options.io, "%soptions: ", padding);

    for (size_t i = 0; i < info->count; i++)
    {
        const cfg_choice_t *choice = info->options + i;
        io_printf(options.io, "%s", choice->text);

        if (i != info->count - 1)
        {
            io_printf(options.io, " | ");
        }
    }

    io_printf(options.io, "\n");
}

static void print_vector(io_t *io, const vector_t *vec)
{
    if (vec == NULL)
    {
        io_printf(io, "(default: empty)\n");
        return;
    }

    size_t len = vector_len(vec);
    if (len == 0)
    {
        io_printf(io, "(default: empty)\n");
        return;
    }

    io_printf(io, "(default: ");

    for (size_t i = 0; i < len; i++)
    {
        const char *str = vector_get(vec, i);
        io_printf(io, "%s", str);

        if (i != len - 1)
        {
            io_printf(io, ", ");
        }
    }

    io_printf(io, ")");
}

static void print_field_default(format_config_t options, const cfg_field_t *field)
{
    switch (cfg_get_type(field))
    {
    case eConfigBool: {
        io_printf(options.io, "(default: %s)\n", cfg_bool_info(field) ? "true" : "false");
        break;
    }

    case eConfigInt: {
        const cfg_int_t *info = cfg_int_info(field);
        io_printf(options.io, "(default: %d)", info->initial);
        print_range(options.io, info->min, info->max);
        io_printf(options.io, "\n");
        break;
    }

    case eConfigVector: {
        const vector_t *info = cfg_vector_info(field);
        print_vector(options.io, info);
        io_printf(options.io, "\n");
        break;
    }

    case eConfigString: {
        const char *info = cfg_string_info(field);
        if (info != NULL)
        {
            io_printf(options.io, "(default: `%s`)", info);
        }
        io_printf(options.io, "\n");
        break;
    }

    case eConfigEnum: {
        print_enum_default(options, field);
        break;
    }

    case eConfigFlags: {
        print_flags_default(options, field);
        break;
    }

    default: {
        const cfg_info_t *info = cfg_get_info(field);
        const char *ty = cfg_type_string(cfg_get_type(field));
        CT_NEVER("unknown field type %s (%s)", ty, info->name);
    }
    }
}

static bool print_field_details(format_config_t options, alignment_info_t alignment, const cfg_field_t *field)
{
    switch (cfg_get_type(field))
    {
    case eConfigEnum: {
        print_enum(options, alignment, field);
        return true;
    }

    case eConfigFlags: {
        print_flags(options, alignment, field);
        return true;
    }

    default:
        break;
    }
    return false;
}

// return true if the field needs a second line
static bool print_field_info(format_config_t options, alignment_info_t alignment, bool win_style, const cfg_field_t *field)
{
    const cfg_info_t *info = cfg_get_info(field);

    size_t offset = print_field_args(options, info, win_style);
    size_t padding = alignment.arg_alignment - offset;

    char *pad = str_repeat(" ", padding, options.arena);

    bool needs_second_line = false;

    // print the first line
    size_t first_newline = str_find(info->brief, "\n");
    if (first_newline == SIZE_MAX)
    {
        io_printf(options.io, "%s%s", pad, info->brief);
        size_t after_brief = alignment.brief_alignment - ctu_strlen(info->brief);
        char *pad2 = str_repeat(" ", after_brief, options.arena);

        io_printf(options.io, "%s", pad2);

        print_field_default(options, field);
    }
    else
    {
        // print the first line
        io_printf(options.io, "%s%.*s", pad, (int)first_newline, info->brief);

        size_t after_brief = alignment.brief_alignment - first_newline;
        char *pad2 = str_repeat(" ", after_brief, options.arena);

        io_printf(options.io, "%s", pad2);

        print_field_default(options, field);

        // print remaining lines, putting padding in front of each
        char *pad_remaining = str_repeat(" ", alignment.arg_alignment, options.arena);
        size_t start = first_newline + 1;
        size_t len = 0;
        for (size_t i = start; info->brief[i]; i++)
        {
            len += 1;
            if (info->brief[i] == '\n')
            {
                io_printf(options.io, "%s%.*s\n", pad_remaining, (int)len, info->brief + start);
                start = i + 1;
                len = 0;
            }
        }

        if (len != 0)
        {
            io_printf(options.io, "%s%.*s\n", pad_remaining, (int)len, info->brief + start);
        }

        needs_second_line = true;
    }

    return print_field_details(options, alignment, field) || needs_second_line;
}

static void print_config_group(format_config_t options, bool win_style, const cfg_group_t *config)
{
    // we right align the args based on the longest
    alignment_info_t alignment = get_group_alignment(config, win_style);

    vector_t *fields = cfg_get_fields(config);
    size_t field_count = vector_len(fields);

    const cfg_info_t *group_info = cfg_group_info(config);

    io_printf(options.io, "%s: %s\n", group_info->name, group_info->brief);

    for (size_t i = 0; i < field_count; i++)
    {
        const cfg_field_t *field = vector_get(fields, i);
        bool needs_second_line = print_field_info(options, alignment, win_style, field);
        if (needs_second_line && i != field_count - 1)
        {
            io_printf(options.io, "\n");
        }
    }

    io_printf(options.io, "\n");
    typevec_t *groups = cfg_get_groups(config);
    size_t group_count = typevec_len(groups);

    for (size_t i = 0; i < group_count; i++)
    {
        const cfg_group_t *group = typevec_offset(groups, i);
        print_config_group(options, win_style, group);
    }
}

static void print_usage(format_config_t options, const char *name)
{
    CTASSERT(name != NULL);

    io_printf(options.io, "usage: %s [options] files...\n\n", name);
    io_printf(options.io,
        " +--- About --------------------------------------------------------------------------+\n"
        " | The command line supports both posix and windows flag syntax                       |\n"
        " | meaning that any flag that can be prefixed with a single dash                      |\n"
        " | can also be prefixed with a forward slash. For example: -h and /h are equivalent.  |\n"
        " | String arguments can be quoted to allow spaces, For example: -o \"output file\"      |\n"
        " | `:`, `=`, or a space can be used to separate the flag from the value               |\n"
        " | -o:output.txt, -o=output.txt, -o output.txt                                        |\n"
        " | A single flag argument may be specified in multiple parts.                         |\n"
        " | For example: /cpp:c++20 /cpp:modules and /cpp:\"c++20,modules\" are all equivilent   |\n"
        " | A leading `-` may be used to disable a flag. For example: /cpp:-modules            |\n"
        " +------------------------------------------------------------------------------------+\n\n"
    );
}

STA_DECL
void print_config(print_config_t print, const cfg_group_t *config)
{
    print_options_t options = print.options;
    format_config_t format_config = {
        .arena = options.arena,
        .io = options.io,
        .context = format_context_make(options)
    };

    if (print.print_usage)
    {
        print_usage(format_config, print.name);
    }

    print_config_group(format_config, print.win_style, config);
}
