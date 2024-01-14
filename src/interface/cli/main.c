#include "config/config.h"
#include "defaults/memory.h"
#include "format/colour.h"
#include "cmd.h"

#include "cthulhu/events/events.h"
#include "io/console.h"
#include "arena/arena.h"
#include "format/notify.h"
#include "scan/node.h"
#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include "argparse/argparse.h"

#include "io/io.h"

#include "fs/fs.h"

#include "notify/notify.h"

#include "cthulhu/ssa/ssa.h"

#include "cthulhu/emit/emit.h"

#include "core/macros.h"
#include "support/langs.h"

static const version_info_t kToolVersion = {
    .license = "GPLv3",
    .desc = "Cthulhu Compiler Collection CLI",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 3),
};

static void parse_source(lifetime_t *lifetime, const char *path)
{
    logger_t *reports = lifetime_get_logger(lifetime);
    arena_t *arena = lifetime_get_arena(lifetime);
    const char *ext = str_ext(path, arena);
    if (ext == NULL)
    {
        msg_notify(reports, &kEvent_NoFileExtension, node_builtin(),
                   "could not identify compiler for `%s` (no extension)", path);
        return;
    }

    const language_t *lang = lifetime_get_language(lifetime, ext);
    if (lang == NULL)
    {
        const char *basepath = str_filename(path, arena);
        event_builder_t id = msg_notify(reports, &kEvent_FailedToIdentifyLanguage, node_builtin(),
                                 "could not identify compiler for `%s` by extension `%s`", basepath,
                                 ext);
        msg_note(id, "extra extensions can be provided with -ext=id:ext");
        return;
    }

    io_t *io = io_file(path, eAccessRead, arena);
    os_error_t err = io_error(io);
    if (err != 0)
    {
        event_builder_t id = msg_notify(reports, &kEvent_FailedToOpenSourceFile, node_builtin(),
                                 "failed to open source `%s`", path);
        msg_note(id, "error: %s", os_error_string(err, arena));
        return;
    }

    lifetime_parse(lifetime, lang, io);
}

static int check_reports(logger_t *logger, report_config_t config, const char *title)
{
    int err = text_report(logger_get_events(logger), config, title);

    if (err != EXIT_OK)
    {
        return err;
    }

    return 0;
}

#define CHECK_LOG(logger, fmt)                               \
    do                                                       \
    {                                                        \
        int err = check_reports(logger, report_config, fmt); \
        if (err != EXIT_OK)                                  \
        {                                                    \
            return err;                                      \
        }                                                    \
    } while (0)

int main(int argc, const char **argv)
{
    default_init();

    arena_t *arena = ctu_default_alloc();
    mediator_t *mediator = mediator_new(arena);
    lifetime_t *lifetime = lifetime_new(mediator, arena);
    logger_t *reports = lifetime_get_logger(lifetime);

    langs_t langs = get_langs();
    for (size_t i = 0; i < langs.size; i++)
    {
        lifetime_add_language(lifetime, langs.langs[i]);
    }

    io_t *con = io_stdout();

    tool_t tool = make_tool(arena);

    tool_config_t config = {
        .arena = arena,
        .io = con,

        .group = tool.config,
        .version = kToolVersion,

        .argc = argc,
        .argv = argv,
    };

    ap_t *ap = ap_new(tool.config, arena);

    int parse_err = parse_argparse(ap, tool.options, config);
    if (parse_err == EXIT_SHOULD_EXIT)
    {
        return EXIT_OK;
    }

    vector_t *paths = ap_get_posargs(ap);

    text_config_t text_config = {
        .config = {
            .zeroth_line = false,
        },
        .colours = &kColourDefault,
        .io = con,
    };

    report_config_t report_config = {
        .report_format = cfg_enum_value(tool.report_style),
        .text_config = text_config,

        .max_errors = cfg_int_value(tool.report_limit),
    };

    CHECK_LOG(reports, "initializing");

    size_t total_sources = vector_len(paths);
    if (total_sources == 0)
    {
        msg_notify(reports, &kEvent_NoSourceFiles, node_builtin(), "no source files provided");
    }

    CHECK_LOG(reports, "opening sources");

    for (size_t i = 0; i < total_sources; i++)
    {
        const char *path = vector_get(paths, i);
        parse_source(lifetime, path);
    }

    CHECK_LOG(reports, "parsing sources");

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = str_format(arena, "running stage %s", stage_to_string(stage));
        CHECK_LOG(reports, msg);
    }

    lifetime_resolve(lifetime);
    CHECK_LOG(reports, "compiling sources");

    map_t *modmap = lifetime_get_modules(lifetime);
    ssa_result_t ssa = ssa_compile(modmap, arena);
    CHECK_LOG(reports, "compiling ssa");

    ssa_opt(reports, ssa);
    CHECK_LOG(reports, "optimizing ssa");

    fs_t *fs = fs_virtual("out", arena);

    emit_options_t base_emit_options = {
        .arena = arena,
        .reports = reports,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    if (cfg_bool_value(tool.emit_ssa))
    {
        ssa_emit_options_t emit_options = {.opts = base_emit_options};

        emit_ssa(&emit_options);
        CHECK_LOG(reports, "emitting ssa");
    }

    c89_emit_options_t c89_emit_options = {.opts = base_emit_options};
    c89_emit_result_t c89_emit_result = emit_c89(&c89_emit_options);
    CTU_UNUSED(c89_emit_result); // TODO: check for errors
    CHECK_LOG(reports, "emitting c89");

    const char *outpath = "out";
    fs_t *out = fs_physical(outpath, arena);
    if (out == NULL)
    {
        msg_notify(reports, &kEvent_FailedToCreateOutputDirectory, node_builtin(),
                   "failed to create output directory `%s`", outpath);
    }

    CHECK_LOG(reports, "creating output directory");

    sync_result_t result = fs_sync(out, fs);
    if (result.path != NULL)
    {
        msg_notify(reports, &kEvent_FailedToWriteOutputFile, node_builtin(), "failed to sync %s",
                   result.path);
    }

    CHECK_LOG(reports, "writing output files");
}
