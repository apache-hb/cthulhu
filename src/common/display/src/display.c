#include "display/display.h"

#include "common.h"

#include "base/panic.h"
#include "core/macros.h"
#include "io/io.h"
#include "config/config.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include <stdio.h>

#include <limits.h>

void display_version(version_display_t options)
{
    CTASSERT(options.name != NULL);

    display_options_t base = options.options;

    version_info_t version = options.version;
    int major = VERSION_MAJOR(version.version);
    int minor = VERSION_MINOR(version.version);
    int patch = VERSION_PATCH(version.version);

    io_printf(base.io, "%s %d.%d.%d\n", options.name, major, minor, patch);
    io_printf(base.io, "written by %s and licensed under %s\n", version.author, version.license);
    io_printf(base.io, "%s\n", version.desc);
}

static size_t get_arg_length(const cfg_info_t *info, size_t long_arg_stride)
{
    size_t len = 0;
    if (info->short_args)
    {
        for (size_t i = 0; info->short_args[i]; i++)
        {
            // +1 for the dash or slash
            // +1 for the space
            len += strlen(info->short_args[i]) + 2;
        }
    }

    if (info->long_args)
    {
        for (size_t i = 0; info->long_args[i]; i++)
        {
            // +1 for the space
            len += strlen(info->long_args[i]) + 1 + long_arg_stride;
        }
    }

    return len;
}

typedef struct alignment_info_t
{
    size_t arg_alignment;
    size_t brief_alignment;
} alignment_info_t;

static alignment_info_t get_group_alignment(const config_t *config, bool win_style)
{
    // +1 for a forward slash, otherwise +2 for 2 dashes
    size_t long_arg_stride = win_style ? 1 : 2;
    size_t longest_brief = 0;

    size_t largest = 0;
    vector_t *fields = cfg_get_fields(config);
    size_t field_count = vector_len(fields);
    for (size_t i = 0; i < field_count; i++)
    {
        const cfg_field_t *field = vector_get(fields, i);
        const cfg_info_t *info = cfg_get_info(field);
        size_t len = get_arg_length(info, long_arg_stride);
        largest = MAX(largest, len);

        size_t brief_len = strlen(info->brief);
        longest_brief = MAX(longest_brief, brief_len);
    }

    alignment_info_t alignment = {
        .arg_alignment = largest,
        .brief_alignment = longest_brief + 1
    };

    return alignment;
}

// print the args for a single field
// returns the number of characters printed
static size_t print_field_args(display_options_t options, const cfg_info_t *info, bool win_style)
{
    const char *short_sep = win_style ? "/" : "-";
    const char *long_sep = win_style ? "/" : "--";

    size_t len = 0;

    if (info->short_args)
    {
        for (size_t i = 0; info->short_args[i]; i++)
        {
            len += io_printf(options.io, "%s%s ", short_sep, info->short_args[i]);
        }
    }

    if (info->long_args)
    {
        for (size_t i = 0; info->long_args[i]; i++)
        {
            len += io_printf(options.io, "%s%s ", long_sep, info->long_args[i]);
        }
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

static void print_enum(display_options_t options, alignment_info_t alignment, const cfg_field_t *field)
{
    const cfg_enum_t *info = cfg_enum_info(field);
    io_printf(options.io, "enum (default: %s)\n", get_enum_option(info->options, info->count, info->initial));

    char *padding = str_repeat(" ", alignment.arg_alignment);
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

static void print_flags(display_options_t options, alignment_info_t alignment, const cfg_field_t *field)
{
    const cfg_flags_t *info = cfg_flags_info(field);
    io_printf(options.io, "flags (default: ");

    bool first = true;
    for (size_t i = 0; i < info->count; i++)
    {
        const cfg_choice_t choice = info->options[i];
        if (info->initial & choice.value)
        {
            if (!first)
            {
                io_printf(options.io, " | ");
            }

            io_printf(options.io, "%s", choice.text);
            first = false;
        }
    }

    io_printf(options.io, ")\n");

    char *padding = str_repeat(" ", alignment.arg_alignment);
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

static void print_field_default(display_options_t options, alignment_info_t alignment, const cfg_field_t *field)
{
    switch (cfg_get_type(field))
    {
    case eConfigBool: {
        const cfg_bool_t *info = cfg_bool_info(field);
        io_printf(options.io, "option (default: %s)\n", info->initial ? "true" : "false");
        break;
    }

    case eConfigInt: {
        const cfg_int_t *info = cfg_int_info(field);
        io_printf(options.io, "int (default: %d)", info->initial);
        print_range(options.io, info->min, info->max);
        io_printf(options.io, "\n");
        break;
    }

    case eConfigString: {
        const cfg_string_t *info = cfg_string_info(field);
        io_printf(options.io, "string (default: `%s`)\n", info->initial);
        break;
    }

    case eConfigEnum: {
        print_enum(options, alignment, field);
        break;
    }

    case eConfigFlags: {
        print_flags(options, alignment, field);
        break;
    }

    default:
        break;
    }
}

static void print_field_info(display_options_t options, alignment_info_t alignment, bool win_style, const cfg_field_t *field)
{
    const cfg_info_t *info = cfg_get_info(field);

    size_t offset = print_field_args(options, info, win_style);
    size_t padding = alignment.arg_alignment - offset;

    char *pad = str_repeat(" ", padding);

    io_printf(options.io, "%s%s", pad, info->brief);

    size_t after_brief = alignment.brief_alignment - strlen(info->brief);
    char *pad2 = str_repeat(" ", after_brief);

    io_printf(options.io, "%s", pad2);

    print_field_default(options, alignment, field);
}

static void print_config_group(display_options_t options, bool win_style, const config_t *config)
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
        print_field_info(options, alignment, win_style, field);
    }

    io_printf(options.io, "\n");
    typevec_t *groups = cfg_get_groups(config);
    size_t group_count = typevec_len(groups);

    for (size_t i = 0; i < group_count; i++)
    {
        const config_t *group = typevec_offset(groups, i);
        print_config_group(options, win_style, group);
    }
}

static void print_usage(display_options_t options, const char *name)
{
    CTASSERT(name != NULL);

    io_printf(options.io, "usage: %s [options] files...\n\n", name);
    io_printf(options.io,
        "the command line supports both posix and windows flag syntax\n"
        "meaning that any flag that can be prefixed with a single dash\n"
        "can also be prefixed with a forward slash\n"
        "for example: -h and /h are equivalent\n\n"
    );
}

void display_config(config_display_t options)
{
    display_options_t base = options.options;

    if (options.print_usage)
    {
        print_usage(base, options.name);
    }

    print_config_group(base, options.win_style, options.config);
}
