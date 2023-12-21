#include "cmd.h"

#include "cthulhu/events/events.h"
#include "memory/memory.h"
#include "notify/colour.h"
#include "notify/text.h"
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
#include <stdio.h>

static const version_info_t kVersionInfo = {
    .license = "GPLv3",
    .desc = "Cthulhu Compiler Collection CLI",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 3),
};

static void parse_source(lifetime_t *lifetime, const char *path)
{
    logger_t *reports = lifetime_get_logger(lifetime);
    const char *ext = str_ext(path);
    if (ext == NULL)
    {
        msg_notify(reports, &kEvent_NoFileExtension, node_builtin(),
                   "could not identify compiler for `%s` (no extension)", path);
        return;
    }

    const language_t *lang = lifetime_get_language(lifetime, ext);
    if (lang == NULL)
    {
        const char *basepath = str_filename(path);
        event_t *id = msg_notify(reports, &kEvent_FailedToIdentifyLanguage, node_builtin(),
                                 "could not identify compiler for `%s` by extension `%s`", basepath,
                                 ext);
        msg_note(id, "extra extensions can be provided with -ext=id:ext");
        return;
    }

    io_t *io = io_file(path, eAccessRead, get_global_arena());
    if (io_error(io) != 0)
    {
        event_t *id = msg_notify(reports, &kEvent_FailedToOpenSourceFile, node_builtin(),
                                 "failed to open source `%s`", path);
        msg_note(id, "error: %s", os_error_string(io_error(io)));
        return;
    }

    lifetime_parse(lifetime, lang, io);
}

static int check_reports(logger_t *logger, report_config_t config, const char *title)
{
    int err = text_report(logger_get_events(logger), config, title);

    text_config_t inner = config.text_config;

    if (err != EXIT_OK)
    {
        const void *buffer = io_map(inner.io);
        size_t size = io_size(inner.io);
        (void)fwrite(buffer, size, 1, stderr);

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
    arena_t *arena = get_global_arena();
    mediator_t *mediator = mediator_new("cli", kVersionInfo);
    lifetime_t *lifetime = lifetime_new(mediator, arena);
    logger_t *reports = lifetime_get_logger(lifetime);

    runtime_t rt = cmd_parse(mediator, lifetime, argc, argv);

    io_t *msg_buffer = io_blob("buffer", 0x1000, eAccessWrite | eAccessText, arena);

    text_config_t text_config = {
        .config = {
            .zeroth_line = false,
            .print_source = true,
            .print_header = true,
        },
        .colours = colour_get_default(),
        .io = msg_buffer,
    };

    report_config_t report_config = {
        .report_format = eTextSimple,
        .text_config = text_config,
    };

    CHECK_LOG(reports, "initializing");

    size_t total_sources = vector_len(rt.sourcePaths);
    if (total_sources == 0)
    {
        msg_notify(reports, &kEvent_NoSourceFiles, node_builtin(), "no source files provided");
    }

    CHECK_LOG(reports, "opening sources");

    for (size_t i = 0; i < total_sources; i++)
    {
        const char *path = vector_get(rt.sourcePaths, i);
        parse_source(lifetime, path);
    }

    CHECK_LOG(reports, "parsing sources");

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_LOG(reports, msg);
    }

    lifetime_resolve(lifetime);
    CHECK_LOG(reports, "compiling sources");

    map_t *modmap = lifetime_get_modules(lifetime);
    ssa_result_t ssa = ssa_compile(modmap);
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

    if (rt.emitSSA)
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
