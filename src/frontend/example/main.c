// SPDX-License-Identifier: GPL-3.0-only

#include "setup/setup.h"
#include "setup/memory.h"
#include "format/colour.h"
#include "format/notify.h"
#include "base/log.h"
#include "cthulhu/events/events.h"
#include "cthulhu/broker/broker.h"

#include "cthulhu/emit/emit.h"
#include "cthulhu/ssa/ssa.h"
#include "cthulhu/check/check.h"

#include "io/console.h"
#include "notify/notify.h"
#include "scan/node.h"
#include "support/loader.h"

#include "arena/arena.h"

#include "std/map.h"
#include "std/str.h"
#include "std/vector.h"

#include "core/macros.h"

#include "fs/fs.h"
#include "io/io.h"

#include "argparse/argparse.h"
#include "support/support.h"

static const frontend_t kFrontendInfo = {
    .info = {
        .id = "frontend/example",
        .name = "Example Frontend",
        .version = {
            .license = "GPLv3",
            .desc = "Example compiler frontend",
            .author = "Elliot Haisley",
            .version = CT_NEW_VERSION(0, 0, 1),
        }
    }
};

static int check_reports(logger_t *logger, report_config_t config, const char *title)
{
    int err = text_report(logger_get_events(logger), config, title);
    logger_reset(logger);

    if (err != CT_EXIT_OK)
    {
        return err;
    }

    return 0;
}

#define CHECK_LOG(logger, fmt)                               \
    do                                                       \
    {                                                        \
        int err = check_reports(logger, report_config, fmt); \
        if (err != CT_EXIT_OK)                                  \
        {                                                    \
            return err;                                      \
        }                                                    \
    } while (0)

static io_t *make_file(logger_t *reports, const char *path, os_access_t flags, arena_t *arena)
{
    io_t *io = io_file(path, flags, arena);
    os_error_t err = io_error(io);
    if (err != 0)
    {
        event_builder_t id = msg_notify(reports, &kEvent_FailedToOpenSourceFile, NULL, "failed to open `%s`", path);
        msg_note(id, "%s", os_error_string(err, arena));
        return NULL;
    }

    return io;
}

int main(int argc, const char **argv)
{
    setup_default(NULL);

    ctu_log_update(true);
    arena_t *arena = ctu_default_alloc();
    node_t *node = node_builtin("example-frontend", arena);
    broker_t *broker = broker_new(&kFrontendInfo, arena);
    logger_t *logger = broker_get_logger(broker);

    loader_t *loader = loader_new(arena);
    support_t *support = support_new(broker, loader, arena);
    support_load_default_modules(support);

    io_t *con = io_stdout();

    text_config_t text_config = {
        .config = {
            .zeroth_line = false,
        },
        .colours = &kColourDefault,
        .io = con,
    };

    report_config_t report_config = {
        .report_format = eTextSimple,
        .text_config = text_config,
    };

    CHECK_LOG(logger, "adding languages");

    for (int i = 1; i < argc; i++)
    {
        const char *path = argv[i];

        // detect language by file extension
        const char *ext = str_ext(path, arena);
        language_runtime_t *lang = support_get_lang(support, ext);
        if (lang == NULL)
        {
            io_printf(con, "no language found for file: %s\n", path);
        }

        io_t *io = make_file(logger, path, eOsAccessRead, arena);
        if (io != NULL)
        {
            broker_parse(lang, io);
        }

        CHECK_LOG(logger, "parsing source");
    }

    for (size_t pass = 0; pass < ePassCount; pass++)
    {
        broker_run_pass(broker, pass);

        char *msg = str_format(arena, "running compilation pass %s", broker_pass_name(pass));
        CHECK_LOG(logger, msg);
    }

    broker_resolve(broker);
    CHECK_LOG(logger, "resolving symbols");

    vector_t *modmap = broker_get_modules(broker);

    check_tree(logger, modmap, arena);
    CHECK_LOG(logger, "checking tree");

    ssa_result_t ssa = ssa_compile(modmap, arena);
    CHECK_LOG(logger, "generating ssa");

    ssa_opt(logger, ssa, arena);
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
    CT_UNUSED(ssa_emit_result); // TODO: check for errors

    c89_emit_options_t c89_emit_options = {.opts = base_options};

    c89_emit_result_t c89_emit_result = emit_c89(&c89_emit_options);
    CHECK_LOG(logger, "emitting c89");

    size_t len = vector_len(c89_emit_result.sources);
    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(c89_emit_result.sources, i);
        io_printf(con, "%s\n", path);
    }

    fs_t *out = fs_physical("out", arena);
    if (out == NULL)
    {
        msg_notify(logger, &kEvent_FailedToCreateOutputDirectory, node, "failed to create output directory");
    }
    CHECK_LOG(logger, "creating output directory");

    sync_result_t result = fs_sync(out, fs);
    if (result.path != NULL)
    {
        msg_notify(logger, &kEvent_FailedToWriteOutputFile, node, "failed to sync %s", result.path);
    }
    CHECK_LOG(logger, "syncing output directory");
}
