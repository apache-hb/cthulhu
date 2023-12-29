#include "config/config.h"
#include "core/macros.h"
#include "defaults/defaults.h"
#include "io/console.h"
#include "io/io.h"
#include "memory/memory.h"

static const version_info_t kVersionInfo = {
    .license = "GPLv3",
    .author = "Elliot Haisley",
    .desc = "Cthulhu language event generator",
    .version = NEW_VERSION(0, 0, 1),
};

static const cfg_info_t kToolInfo = {
    .name = "events",
    .brief = "Cthulhu language event generator",
};

static const char *const kSetupInShortArgs[] = {"setup", NULL};

static const cfg_info_t kSetupInInfo = {
    .name = "setup",
    .brief = "The setup file to use",
    .short_args = kSetupInShortArgs,
};

static const char *const kPrefixShortArgs[] = {"prefix", NULL};

static const cfg_info_t kPrefixInfo = {
    .name = "prefix",
    .brief = "The prefix to use for the generated events",
    .short_args = kPrefixShortArgs,
};

static const char *const kSourceOutLongArgs[] = {"source", NULL};

static const cfg_info_t kSourceOutInfo = {
    .name = "source",
    .brief = "The output file to write the source to",
    .long_args = kSourceOutLongArgs,
};

static const char *const kHeaderOutLongArgs[] = {"header", NULL};

static const cfg_info_t kHeaderOutInfo = {
    .name = "header",
    .brief = "The output file to write the header to",
    .long_args = kHeaderOutLongArgs,
};

typedef struct tool_t
{
    config_t *config;

    cfg_field_t *setup_in;
    cfg_field_t *prefix_in;

    cfg_field_t *source_out;
    cfg_field_t *header_out;

    default_options_t options;
} tool_t;

static tool_t make_tool(arena_t *arena)
{
    config_t *root = config_new(arena, &kToolInfo);

    cfg_string_t setup_in_initial = { .initial = NULL };
    cfg_field_t *setup_in = config_string(root, &kSetupInInfo, setup_in_initial);

    cfg_string_t prefix_in_initial = { .initial = NULL };
    cfg_field_t *prefix_in = config_string(root, &kPrefixInfo, prefix_in_initial);

    cfg_string_t source_out_initial = { .initial = NULL };
    cfg_field_t *source_out = config_string(root, &kSourceOutInfo, source_out_initial);

    cfg_string_t header_out_initial = { .initial = NULL };
    cfg_field_t *header_out = config_string(root, &kHeaderOutInfo, header_out_initial);

    default_options_t options = get_default_options(root);

    tool_t tool = {
        .config = root,

        .setup_in = setup_in,
        .prefix_in = prefix_in,

        .source_out = source_out,
        .header_out = header_out,

        .options = options,
    };

    return tool;
}

int main(int argc, const char **argv)
{
    default_init();
    arena_t *arena = get_global_arena();
    io_t *io = io_stdout();

    tool_t tool = make_tool(arena);

    tool_config_t config = {
        .arena = arena,
        .io = io,

        .group = tool.config,
        .version = kVersionInfo,

        .argc = argc,
        .argv = argv,
    };

    int err = parse_commands(tool.options, config);
    if (err == EXIT_SHOULD_EXIT)
    {
        return EXIT_OK;
    }

    const char *setup_in = cfg_string_value(tool.setup_in);
    const char *source_out = cfg_string_value(tool.source_out);
    const char *header_out = cfg_string_value(tool.header_out);

    if (setup_in == NULL)
    {
        io_printf(io, "no setup file provided\n");
        return EXIT_ERROR;
    }

    if (source_out == NULL)
    {
        io_printf(io, "no source file provided\n");
        return EXIT_ERROR;
    }

    if (header_out == NULL)
    {
        io_printf(io, "no header file provided\n");
        return EXIT_ERROR;
    }

    io_t *source_io = io_file_arena(source_out, eAccessWrite | eAccessText, arena);
    os_error_t source_err = io_error(source_io);
    if (source_err)
    {
        io_printf(io, "failed to open source file `%s` %s\n", source_out, os_error_string(source_err));
        return EXIT_ERROR;
    }

    io_printf(source_io, "#include \"%s\"\n\n", header_out);
    io_printf(source_io, "#define NEW_EVENT(id, ...) const diagnostic_t kEvent##_id = __VA_ARGS__;\n");
    io_printf(source_io, "#include \"%s\"\n\n", setup_in);

    io_t *header_io = io_file_arena(header_out, eAccessWrite | eAccessText, arena);
    os_error_t header_err = io_error(header_io);
    if (header_err)
    {
        io_printf(io, "failed to open header file `%s` %s\n", header_out, os_error_string(header_err));
        return EXIT_ERROR;
    }

    io_printf(header_io, "#pragma once\n\n");
    io_printf(header_io, "#include \"notify/diagnostic.h\"\n\n");
    io_printf(header_io, "#define NEW_EVENT(id, ...) extern const diagnostic_t kEvent##_id;\n");
    io_printf(header_io, "#include \"%s\"\n\n", setup_in);

    io_close(source_io);
    io_close(header_io);

    return EXIT_OK;
}
