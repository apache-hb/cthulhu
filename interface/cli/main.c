#include "cmd.h"

#include "cthulhu/events/events.h"
#include "memory/memory.h"
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

#define CHECK_REPORTS(reports)                                                                     \
    do                                                                                             \
    {                                                                                              \
        int err = text_report(logger_get_events(reports), reportConfig);                           \
        if (err != 0)                                                                              \
        {                                                                                          \
            return err;                                                                            \
        }                                                                                          \
    } while (0)

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

    io_t *io = io_file(path, eAccessRead);
    if (io_error(io) != 0)
    {
        event_t *id = msg_notify(reports, &kEvent_FailedToOpenSourceFile, node_builtin(),
                                 "failed to open source `%s`", path);
        msg_note(id, "error: %s", os_error_string(io_error(io)));
        return;
    }

    lifetime_parse(lifetime, lang, io);
}

int main(int argc, const char **argv)
{
    mediator_t *mediator = mediator_new("cli", kVersionInfo);
    lifetime_t *lifetime = lifetime_new(mediator, ctu_default_alloc());
    logger_t *reports = lifetime_get_logger(lifetime);

    runtime_t rt = cmd_parse(mediator, lifetime, argc, argv);

    vector_t *errors = logger_get_events(reports);
    size_t len = vector_len(errors);
    if (len > 0)
    {
        for (size_t i = 0; i < len; i++)
        {
            event_t *event = vector_get(errors, i);
            printf("%s\n", event->message); // TODO: use text reporting
        }

        return 1;
    }

    // CHECK_REPORTS(reports, "failed to parse command line arguments");

    size_t totalSources = vector_len(rt.sourcePaths);
    if (totalSources == 0)
    {
        msg_notify(reports, &kEvent_NoSourceFiles, node_builtin(), "no source files provided");
        // report(reports, eFatal, node_builtin(), "no source files provided");
    }

    // CHECK_REPORTS(reports, "failed to load sources");

    for (size_t i = 0; i < totalSources; i++)
    {
        const char *path = vector_get(rt.sourcePaths, i);
        parse_source(lifetime, path);
    }

    // CHECK_REPORTS(reports, "failed to parse sources");

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        // CHECK_REPORTS(reports, msg);
    }

    lifetime_resolve(lifetime);
    // CHECK_REPORTS(reports, "resolving symbols");

    map_t *modmap = lifetime_get_modules(lifetime);
    ssa_result_t ssa = ssa_compile(modmap);
    // CHECK_REPORTS(reports, "generating ssa");

    ssa_opt(reports, ssa);
    // CHECK_REPORTS(reports, "optimizing ssa");

    fs_t *fs = fs_virtual("out");

    emit_options_t baseOpts = {
        .reports = reports,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    if (rt.emitSSA)
    {
        ssa_emit_options_t emitOpts = {.opts = baseOpts};

        emit_ssa(&emitOpts);
        // CHECK_REPORTS(reports, "emitting ssa");
    }

    c89_emit_options_t c89Opts = {.opts = baseOpts};
    c89_emit_result_t c89Result = emit_c89(&c89Opts);
    CTU_UNUSED(c89Result); // TODO: check for errors
    // CHECK_REPORTS(reports, "emitting c89");

    const char *outpath = "out";
    fs_t *out = fs_physical(outpath);
    if (out == NULL)
    {
        msg_notify(reports, &kEvent_FailedToCreateOutputDirectory, node_builtin(),
                   "failed to create output directory `%s`", outpath);
        // report(reports, eFatal, node_builtin(), "failed to create output directory");
    }

    // CHECK_REPORTS(reports, "creating output directory");

    sync_result_t result = fs_sync(out, fs);
    if (result.path != NULL)
    {
        msg_notify(reports, &kEvent_FailedToWriteOutputFile, node_builtin(), "failed to sync %s",
                   result.path);
        // report(reports, eFatal, NULL, "failed to sync %s", result.path);
    }
    // CHECK_REPORTS(reports, "syncing output directory");

}
