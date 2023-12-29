#include "argparse/argparse.h"
#include "base/log.h"
#include "config/config.h"
#include "core/macros.h"
#include "defaults/defaults.h"
#include "io/console.h"
#include "io/io.h"
#include "memory/memory.h"
#include "std/vector.h"
#include "notify/notify.h"
#include "scan/scan.h"

#include "cpp/cpp.h"

static const version_info_t kVersionInfo = {
    .license = "GPLv3",
    .author = "Elliot Haisley",
    .desc = "C language preprocessor",
    .version = NEW_VERSION(0, 0, 1),
};

static const cfg_info_t kToolInfo = {
    .name = "events",
    .brief = "Preprocesses C code",
};

static const char *const kIncludeDirShortArgs[] = {"I", NULL};

static const cfg_info_t kIncludeDirInfo = {
    .name = "include-dir",
    .brief = "Add an include directory",
    .short_args = kIncludeDirShortArgs,
};

typedef struct tool_t
{
    config_t *config;

    cfg_field_t *include_dirs;

    default_options_t options;
} tool_t;

static tool_t make_tool(arena_t *arena)
{
    config_t *root = config_new(arena, &kToolInfo);

    cfg_string_t include_dirs_initial = { .initial = NULL };
    cfg_field_t *include_dirs = config_string(root, &kIncludeDirInfo, include_dirs_initial);

    default_options_t options = get_default_options(root);

    tool_t tool = {
        .config = root,

        .include_dirs = include_dirs,

        .options = options,
    };

    return tool;
}

int main(int argc, const char **argv)
{
    default_init();
    ctu_log_update(true);

    arena_t *arena = get_global_arena();
    io_t *con = io_stdout();

    for (int i = 0; i < argc; i++)
    {
        io_printf(con, "%d: %s\n", i, argv[i]);
    }

    tool_t tool = make_tool(arena);

    tool_config_t config = {
        .arena = arena,
        .io = con,

        .group = tool.config,
        .version = kVersionInfo,

        .argc = argc,
        .argv = argv,
    };

    ap_t *ap = ap_new(tool.config, arena);

    int err = parse_argparse(ap, tool.options, config);
    if (err == EXIT_SHOULD_EXIT)
    {
        return EXIT_OK;
    }

    vector_t *pos = ap_get_posargs(ap);
    size_t len = vector_len(pos);
    if (len == 0)
    {
        io_printf(con, "No input files\n");
        return EXIT_OK;
    }

    logger_t *logger = logger_new(arena);

    cpp_instance_t instance = cpp_instance_new(arena, logger);

    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(pos, i);
        io_printf(con, "Processing `%s`\n", path);

        io_t *io = io_file_arena(path, eAccessRead | eAccessText, arena);
        scan_t *scan = scan_io("C", io, arena);

        io_t *result = cpp_process_file(&instance, scan);

        size_t size = io_size(result);
        const char *text = io_map(result);

        io_printf(con, "%.*s", (int)size, text);
    }

    return EXIT_OK;
}
