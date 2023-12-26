#include "argparse/argparse.h"
#include "base/colour.h"
#include "base/panic.h"
#include "core/macros.h"
#include "defaults/defaults.h"
#include "display/display.h"
#include "io/io.h"
#include "memory/memory.h"
#include "os/os.h"
#include "stacktrace/stacktrace.h"

#include "config/config.h"
#include "std/vector.h"

static const cfg_info_t kGroupInfo = {
    .name = "general",
    .brief = "General options",
};

static const char *const kHelpInfoShortArgs[] = {"h", "?", NULL};
static const char *const kHelpInfoLongArgs[] = {"help", NULL};

static const cfg_info_t kHelpInfo = {
    .name = "help",
    .brief = "Print this help message and exit",
    .short_args = kHelpInfoShortArgs,
    .long_args = kHelpInfoLongArgs,
};

static const char *const kVersionInfoShortArgs[] = {"v", NULL};
static const char *const kVersionInfoLongArgs[] = {"version", NULL};

static const cfg_info_t kVersionInfo = {
    .name = "version",
    .brief = "Print version information and exit",
    .short_args = kVersionInfoShortArgs,
    .long_args = kVersionInfoLongArgs,
};

static const char *const kArgparseUsageInfoShortArgs[] = {"u", NULL};
static const char *const kArgparseUsageInfoLongArgs[] = {"argparse-usage", NULL};

static const cfg_info_t kArgparseUsageInfo = {
    .name = "argparse-usage",
    .brief = "Print argparse usage information",
    .short_args = kArgparseUsageInfoShortArgs,
    .long_args = kArgparseUsageInfoLongArgs,
};

static const char *const kWindowsStyleInfoShortArgs[] = {"w", NULL};
static const char *const kWindowsStyleInfoLongArgs[] = {"windows-style", NULL};

static const cfg_info_t kWindowsStyleInfo = {
    .name = "windows-style",
    .brief = "Enable windows style help output\nprints /flags instead of -flags",
    .short_args = kWindowsStyleInfoShortArgs,
    .long_args = kWindowsStyleInfoLongArgs,
};

static const char *const kColourInfoShortArgs[] = {"c", NULL};
static const char *const kColourInfoLongArgs[] = {"colour", NULL};

static const cfg_info_t kColourInfo = {
    .name = "colour",
    .brief = "Enable colour output",
    .short_args = kColourInfoShortArgs,
    .long_args = kColourInfoLongArgs,
};

default_options_t get_default_options(config_t *group)
{
    CTASSERT(group != NULL);

    config_t *general = config_group(group, &kGroupInfo);

    cfg_bool_t help_initial = { .initial = false };
    cfg_field_t *help = config_bool(general, &kHelpInfo, help_initial);

    cfg_bool_t version_initial = { .initial = false };
    cfg_field_t *version = config_bool(general, &kVersionInfo, version_initial);

    cfg_bool_t argparse_usage_initial = { .initial = false };
    cfg_field_t *argparse_usage = config_bool(general, &kArgparseUsageInfo, argparse_usage_initial);

    cfg_bool_t windows_style_initial = { .initial = DISPLAY_WIN_STYLE };
    cfg_field_t *windows_style = config_bool(general, &kWindowsStyleInfo, windows_style_initial);

    cfg_bool_t colour_initial = { .initial = false };
    cfg_field_t *colour = config_bool(general, &kColourInfo, colour_initial);

    default_options_t options = {
        .group = general,
        .print_help = help,
        .print_version = version,
        .enable_usage = argparse_usage,
        .enable_windows_style = windows_style,
        .colour_output = colour,
    };

    return options;
}

int process_default_options(default_options_t options, tool_config_t config)
{
    const char *name = config.argv[0];

    bool colour = cfg_bool_value(options.colour_output);
    display_options_t display = {
        .arena = config.arena,
        .io = config.io,
        .colours = colour ? &kColourDefault : &kColourNone,
    };

    bool print_help = cfg_bool_value(options.print_help);
    if (print_help)
    {
        config_display_t config_display = {
            .options = display,
            .config = config.group,
            .print_usage = cfg_bool_value(options.enable_usage),
            .win_style = cfg_bool_value(options.enable_windows_style),
            .name = name
        };

        display_config(config_display);
        return EXIT_SHOULD_EXIT;
    }

    bool print_version = cfg_bool_value(options.print_version);
    if (print_version)
    {
        version_display_t version_display = {
            .options = display,
            .version = config.version,
            .name = name
        };

        display_version(version_display);
        return EXIT_SHOULD_EXIT;
    }

    return EXIT_OK;
}

static int process_argparse_result(default_options_t options, tool_config_t config, ap_t *ap)
{
    int err = ap_parse(ap, config.argc, config.argv);

    size_t count = ap_count_params(ap);
    vector_t *posargs = ap_get_posargs(ap);
    size_t posarg_count = vector_len(posargs);

    bool has_no_args = count == 0 && posarg_count == 0;

    if (!has_no_args && err == EXIT_OK)
    {
        return EXIT_OK;
    }

    bool colour = cfg_bool_value(options.colour_output);
    const colour_pallete_t *pallete = colour ? &kColourDefault : &kColourNone;

    if (has_no_args)
    {
        io_printf(config.io, "no arguments provided\n");
    }

    config_display_t display = {
        .options = {
            .arena = config.arena,
            .io = config.io,
            .colours = pallete,
        },
        .config = config.group,
        .print_usage = cfg_bool_value(options.enable_usage),
        .win_style = cfg_bool_value(options.enable_windows_style),
        .name = config.argv[0]
    };

    display_config(display);
    return EXIT_SHOULD_EXIT;
}

int parse_commands(default_options_t options, tool_config_t config)
{
    ap_t *ap = ap_new(config.group, config.arena);
    int err = process_argparse_result(options, config, ap);
    if (err != EXIT_OK)
    {
        return err;
    }

    return process_default_options(options, config);
}

void default_init(void)
{
    bt_init();
    os_init();

    arena_t *arena = ctu_default_alloc();
    init_global_arena(arena);
    init_gmp_arena(arena);
}
