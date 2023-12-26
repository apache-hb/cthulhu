#include "argparse/argparse.h"
#include "base/colour.h"
#include "config/config.h"

#include "core/macros.h"
#include "display/display.h"
#include "io/console.h"
#include "memory/memory.h"
#include "os/os.h"
#include "stacktrace/stacktrace.h"

static const version_info_t kToolVersion = {
    .license = "GPLv3",
    .desc = "Display format testing tool",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1),
};

typedef struct tool_t
{
    config_t *config;

    cfg_field_t *print_help;
    cfg_field_t *print_version;
    cfg_field_t *print_argparse_usage;
    cfg_field_t *enable_colour;
    cfg_field_t *enable_header;
    cfg_field_t *enable_windows_style;

    cfg_field_t *enum_argument;
    cfg_field_t *flag_argument;
} tool_t;

static const cfg_info_t kToolInfo = {
    .name = "display",
    .brief = "Display format testing tool"
};

static const char *const kHelpInfoShortArgs[] = {"h", "?", NULL};
static const char *const kHelpInfoLongArgs[] = {"help", NULL};

static const cfg_info_t kHelpInfo = {
    .name = "help",
    .brief = "Print help message",
    .short_args = kHelpInfoShortArgs,
    .long_args = kHelpInfoLongArgs,
};

static const char *const kVersionInfoShortArgs[] = {"v", NULL};
static const char *const kVersionInfoLongArgs[] = {"version", NULL};

static const cfg_info_t kVersionInfo = {
    .name = "version",
    .brief = "Print version information",
    .short_args = kVersionInfoShortArgs,
    .long_args = kVersionInfoLongArgs,
};

static const char *const kArgparseUsageInfoShortArgs[] = {"u",NULL};
static const char *const kArgparseUsageInfoLongArgs[] = {"argparse-usage",NULL};

static const cfg_info_t kArgparseUsageInfo = {
    .name = "argparse-usage",
    .brief = "Print argparse usage information",
    .short_args = kArgparseUsageInfoShortArgs,
    .long_args = kArgparseUsageInfoLongArgs,
};

static const char *const kWindowsStyleInfoShortArgs[] = {"w",NULL};
static const char *const kWindowsStyleInfoLongArgs[] = {"windows-style",NULL};

static const cfg_info_t kWindowsStyleInfo = {
    .name = "windows-style",
    .brief = "Enable windows style output",
    .short_args = kWindowsStyleInfoShortArgs,
    .long_args = kWindowsStyleInfoLongArgs,
};

static const char *const kColourInfoShortArgs[] = {"c",NULL};
static const char *const kColourInfoLongArgs[] = {"colour",NULL};

static const cfg_info_t kColourInfo = {
    .name = "colour",
    .brief = "Enable colour output",
    .short_args = kColourInfoShortArgs,
    .long_args = kColourInfoLongArgs,
};

static const char *const kHeaderInfoShortArgs[] = {"H",NULL};
static const char *const kHeaderInfoLongArgs[] = {"header",NULL};

static const cfg_info_t kHeaderInfo = {
    .name = "header",
    .brief = "Enable header output",
    .short_args = kHeaderInfoShortArgs,
    .long_args = kHeaderInfoLongArgs,
};

enum arg_option_t
{
    eOptionOne = 1,
    eOptionTwo = 2,
    eOptionThree = 3,
};

static const cfg_choice_t kArgChoices[] = {
    {"one", eOptionOne},
    {"two", eOptionTwo},
    {"three", eOptionThree},
};

static const char *const kEnumInfoShortArgs[] = {"e",NULL};
static const char *const kEnumInfoLongArgs[] = {"enum",NULL};

static const cfg_info_t kEnumInfo = {
    .name = "enum",
    .brief = "An enum argument",
    .short_args = kEnumInfoShortArgs,
    .long_args = kEnumInfoLongArgs
};

enum flag_option_t
{
    eFlagOne = (1 << 0),
    eFlagTwo = (1 << 1),
    eFlagThree = (1 << 2),
};

static const cfg_choice_t kFlagChoices[] = {
    {"one", eFlagOne},
    {"two", eFlagTwo},
    {"three", eFlagThree},
};

static const char *const kFlagInfoShortArgs[] = {"f",NULL};
static const char *const kFlagInfoLongArgs[] = {"flag",NULL};

static const cfg_info_t kFlagInfo = {
    .name = "flag",
    .brief = "A flag argument",
    .short_args = kFlagInfoShortArgs,
    .long_args = kFlagInfoLongArgs
};

static tool_t make_config(arena_t *arena)
{
    config_t *config = config_new(arena, &kToolInfo);

    cfg_bool_t print_init = { .initial = false };
    cfg_field_t *print_help = config_bool(config, &kHelpInfo, print_init);

    cfg_bool_t version_init = { .initial = false };
    cfg_field_t *print_version = config_bool(config, &kVersionInfo, version_init);

    cfg_bool_t colour_init = { .initial = false };
    cfg_field_t *enable_colour = config_bool(config, &kColourInfo, colour_init);

    cfg_bool_t argparse_init = { .initial = false };
    cfg_field_t *print_argparse_usage = config_bool(config, &kArgparseUsageInfo, argparse_init);

    cfg_bool_t windows_init = { .initial = false };
    cfg_field_t *enable_windows_style = config_bool(config, &kWindowsStyleInfo, windows_init);

    cfg_bool_t header_init = { .initial = false };
    cfg_field_t *enable_header = config_bool(config, &kHeaderInfo, header_init);

    cfg_enum_t enum_init = {
        .options = kArgChoices,
        .count = (sizeof(kArgChoices) / sizeof(cfg_choice_t)),
        .initial = eOptionOne
    };
    cfg_field_t *enum_argument = config_enum(config, &kEnumInfo, enum_init);

    cfg_flags_t flag_init = {
        .options = kFlagChoices,
        .count = (sizeof(kFlagChoices) / sizeof(cfg_choice_t)),
        .initial = eFlagOne
    };
    cfg_field_t *flag_argument = config_flags(config, &kFlagInfo, flag_init);

    tool_t tool = {
        .config = config,
        .print_help = print_help,
        .print_version = print_version,
        .enable_colour = enable_colour,
        .enable_header = enable_header,
        .print_argparse_usage = print_argparse_usage,
        .enable_windows_style = enable_windows_style,
        .enum_argument = enum_argument,
        .flag_argument = flag_argument,
    };

    return tool;
}

int main(int argc, const char **argv)
{
    bt_init();
    os_init();

    arena_t *arena = ctu_default_alloc();
    init_global_arena(arena);

    io_t *io = io_stdout(arena);

    display_options_t options = {
        .arena = arena,
        .io = io,
        .colours = &kColourNone,
    };

    tool_t tool_config = make_config(arena);
    ap_t *ap = ap_new(tool_config.config, arena);
    int err = ap_parse(ap, argc, argv);

    config_display_t config_display = {
        .options = options,
        .config = tool_config.config,
        .print_usage = cfg_bool_value(tool_config.print_argparse_usage),
        .win_style = cfg_bool_value(tool_config.enable_windows_style),
        .name = argv[0]
    };

    bool print_help = cfg_bool_value(tool_config.print_help);
    bool print_version = cfg_bool_value(tool_config.print_version);

    if (cfg_bool_value(tool_config.enable_colour))
    {
        options.colours = &kColourDefault;
    }

    if (!print_help && !print_version)
    {
        display_config(config_display);
        return EXIT_OK;
    }

    if (err != EXIT_OK)
    {
        display_config(config_display);
        return err;
    }
    else if (print_help)
    {
        display_config(config_display);
        return EXIT_OK;
    }

    version_display_t version_display = {
        .options = options,
        .version = kToolVersion,
        .name = argv[0]
    };

    if (print_version)
    {
        display_version(version_display);
        return EXIT_OK;
    }
}
