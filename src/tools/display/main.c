#include "config/config.h"

#include "core/macros.h"
#include "setup/setup.h"
#include "io/console.h"
#include "memory/memory.h"

static const version_info_t kToolVersion = {
    .license = "GPLv3",
    .desc = "Display format testing tool",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1),
};

typedef struct tool_t
{
    cfg_group_t *m_config;

    cfg_field_t *enum_argument;
    cfg_field_t *flag_argument;

    default_options_t m_options;
} tool_t;

static const cfg_info_t kToolInfo = {
    .name = "display",
    .brief = "Display format testing tool"
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

static const char *const kEnumInfoShortArgs[] = {"e", NULL};
static const char *const kEnumInfoLongArgs[] = {"enum", NULL};

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

static const char *const kFlagInfoShortArgs[] = {"f", NULL};
static const char *const kFlagInfoLongArgs[] = {"flag", NULL};

static const cfg_info_t kFlagInfo = {
    .name = "flag",
    .brief = "A flag argument",
    .short_args = kFlagInfoShortArgs,
    .long_args = kFlagInfoLongArgs
};

static tool_t make_config(arena_t *arena)
{
    cfg_group_t *config = config_root(&kToolInfo, arena);

    default_options_t options = get_default_options(config);

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
        .m_config = config,
        .enum_argument = enum_argument,
        .flag_argument = flag_argument,

        .m_options = options,
    };

    return tool;
}

int main(int argc, const char **argv)
{
    setup_global();
    arena_t *arena = get_global_arena();

    io_t *io = io_stdout();

    tool_t tool = make_config(arena);

    tool_config_t config = {
        .arena = arena,
        .io = io,

        .group = tool.m_config,
        .version = kToolVersion,

        .argc = argc,
        .argv = argv,
    };

    int err = parse_commands(tool.m_options, config);
    if (err == CT_EXIT_SHOULD_EXIT)
    {
        return CT_EXIT_OK;
    }
}
