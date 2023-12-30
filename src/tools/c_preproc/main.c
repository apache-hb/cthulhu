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

// .\build\src\tools\c_preproc\preproc_tool.exe "C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\Windows.h" /I:"C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\shared"

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

    cfg_field_t *include_dirs = config_vector(root, &kIncludeDirInfo, NULL);

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

    instance.include_depth = 8;

    // TODO: check if include directories exist

    vector_t *include_dirs = cfg_vector_value(tool.include_dirs);
    instance.include_directories = include_dirs;

    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(pos, i);
        io_printf(con, "Processing `%s`\n", path);

        io_t *io = io_file_arena(path, eAccessRead | eAccessText, arena);
        os_error_t os_err = io_error(io);
        if (os_err != 0)
        {
            io_close(io);
            io_printf(con, "Failed to open `%s`\n", path);
            continue;
        }

        scan_t *scan = scan_io("C", io, arena);

        io_t *result = cpp_process_file(&instance, scan);

        size_t size = io_size(result);
        const char *text = io_map(result);

        io_printf(con, "%.*s", (int)size, text);
    }

    return EXIT_OK;
}
