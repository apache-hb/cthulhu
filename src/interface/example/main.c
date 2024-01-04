#include "format/colour.h"
#include "format/notify.h"
#include "base/log.h"
#include "cthulhu/events/events.h"
#include "cthulhu/runtime/interface.h"

#include "cthulhu/emit/emit.h"
#include "cthulhu/ssa/ssa.h"

#include "notify/notify.h"
#include "scan/node.h"
#include "support/langs.h"

#include "memory/memory.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include "core/macros.h"

#include "fs/fs.h"
#include "io/io.h"

#include "argparse/argparse.h"

#include <stdio.h>

// static const version_info_t kVersion = {
//     .license = "GPLv3",
//     .desc = "Example compiler interface",
//     .author = "Elliot Haisley",
//     .version = NEW_VERSION(0, 0, 1),
// };

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

static io_t *make_file(logger_t *reports, const char *path, os_access_t flags)
{
    io_t *io = io_file(path, flags);
    if (io_error(io) != 0)
    {
        event_t *id = msg_notify(reports, &kEvent_FailedToOpenSourceFile, NULL, "failed to open `%s`", path);
        msg_note(id, "%s", os_error_string(io_error(io)));
        return NULL;
    }

    return io;
}

int main(int argc, const char **argv)
{
    ctu_log_update(true);
    arena_t *arena = ctu_default_alloc();
    mediator_t *mediator = mediator_new(arena);
    lifetime_t *lifetime = lifetime_new(mediator, arena);
    logger_t *logger = lifetime_get_logger(lifetime);

    langs_t langs = get_langs();
    for (size_t i = 0; i < langs.size; i++)
    {
        lifetime_add_language(lifetime, langs.langs[i]);
    }

    io_t *msg_buffer = io_blob("buffer", 0x1000, eAccessWrite | eAccessText, arena);

    text_config_t text_config = {
        .config = {
            .zeroth_line = false,
            .print_source = true,
            .print_header = true,
        },
        .colours = &kColourDefault,
        .io = msg_buffer,
    };

    report_config_t report_config = {
        .report_format = eTextSimple,
        .text_config = text_config,
    };

    CHECK_LOG(logger, "adding languages");

    for (int i = 1; i < argc; i++)
    {
        const char *path = argv[i];
        const char *ext = str_ext(path);
        const language_t *lang = lifetime_get_language(lifetime, ext);

        if (lang == NULL)
        {
            printf("no language found for file: %s\n", path);
        }

        io_t *io = make_file(logger, path, eAccessRead | eAccessText);
        if (io != NULL)
        {
            lifetime_parse(lifetime, lang, io);
        }

        CHECK_LOG(logger, "parsing source");
    }

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_LOG(logger, msg);
    }

    lifetime_resolve(lifetime);
    CHECK_LOG(logger, "resolving symbols");

    map_t *modmap = lifetime_get_modules(lifetime);

    ssa_result_t ssa = ssa_compile(modmap);
    CHECK_LOG(logger, "generating ssa");

    ssa_opt(logger, ssa);
    CHECK_LOG(logger, "optimizing ssa");

    fs_t *fs = fs_virtual("out", arena);

    emit_options_t base_options = {
        .arena = arena,
        .reports = logger,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    ssa_emit_options_t ssa_emit_options = {.opts = base_options};

    ssa_emit_result_t ssa_emit_result = emit_ssa(&ssa_emit_options);
    CHECK_LOG(logger, "emitting ssa");
    CTU_UNUSED(ssa_emit_result); // TODO: check for errors

    c89_emit_options_t c89_emit_options = {.opts = base_options};

    c89_emit_result_t c89_emit_result = emit_c89(&c89_emit_options);
    CHECK_LOG(logger, "emitting c89");

    size_t len = vector_len(c89_emit_result.sources);
    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(c89_emit_result.sources, i);
        printf("%s\n", path);
    }

    fs_t *out = fs_physical("out", arena);
    if (out == NULL)
    {
        msg_notify(logger, &kEvent_FailedToCreateOutputDirectory, node_builtin(), "failed to create output directory");
    }
    CHECK_LOG(logger, "creating output directory");

    sync_result_t result = fs_sync(out, fs);
    if (result.path != NULL)
    {
        msg_notify(logger, &kEvent_FailedToWriteOutputFile, node_builtin(), "failed to sync %s", result.path);
    }
    CHECK_LOG(logger, "syncing output directory");
}
