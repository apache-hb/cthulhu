#include "cmd.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "argparse/argparse.h"
#include "argparse/commands.h"

#include "io/io.h"
#include "io/fs.h"

#include "report/report.h"

#include "cthulhu/ssa/ssa.h"

#include "cthulhu/emit/c89.h"

#include "support/langs.h"

static const version_info_t kVersionInfo = {
    .license = "GPLv3",
    .desc = "Cthulhu Compiler Collection CLI",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 2)
};

#define CHECK_REPORTS(reports, msg) \
    do { \
        int err = end_reports(reports, msg, reportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

static void parse_source(lifetime_t *lifetime, const char *path)
{
    reports_t *reports = lifetime_get_reports(lifetime);
    const char *ext = str_ext(path);
    if (ext == NULL)
    {
        report(reports, eFatal, NULL, "could not identify compiler for `%s` (no extension)", path);
        return;
    }

    const language_t *lang = lifetime_get_language(lifetime, ext);
    if (lang == NULL)
    {
        const char *basepath = str_filename(path);
        message_t *id = report(reports, eFatal, NULL, "could not identify compiler for `%s` by extension `%s`", basepath, ext);
        report_note(id, "extra extensions can be provided with -ext=id:ext");
        return;
    }

    const char *cwd = get_cwd();

    io_t *io = io_file(format("%s" NATIVE_PATH_SEPARATOR "%s", cwd, path), eFileRead | eFileText);
    if (io_error(io) != 0)
    {
        report(reports, eFatal, NULL, "failed to load source `%s`\n%s", path, error_string(io_error(io)));
        return;
    }

    lifetime_parse(lifetime, lang, io);
}

int main(int argc, const char **argv)
{
    mediator_t *mediator = mediator_new("cli", kVersionInfo);
    lifetime_t *lifetime = lifetime_new(mediator);
    reports_t *reports = lifetime_get_reports(lifetime);

    runtime_t rt = cmd_parse(reports, mediator, lifetime, argc, argv);
    report_config_t reportConfig = {
        .limit = rt.reportLimit,
        .warningsAreErrors = rt.warnAsError
    };

    CHECK_REPORTS(reports, "failed to parse command line arguments");

    size_t len = vector_len(rt.sourcePaths);
    if (len == 0)
    {
        report(reports, eFatal, NULL, "no source files provided");
    }

    CHECK_REPORTS(reports, "failed to load sources");

    for (size_t i = 0; i < len; i++)
    {
        const char *path = vector_get(rt.sourcePaths, i);
        parse_source(lifetime, path);
    }

    CHECK_REPORTS(reports, "failed to parse sources");

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_REPORTS(reports, msg);
    }

    vector_t *mods = lifetime_get_modules(lifetime);
    ssa_module_t *ssa = ssa_gen_module(reports, mods);
    CHECK_REPORTS(reports, "generating ssa");

    ssa_opt_module(reports, ssa);
    CHECK_REPORTS(reports, "failed to optimize SSA");

    if (rt.emitSSA)
    {
        ssa_emit_module(reports, ssa);
    }

    c89_emit_t config = {
        .reports = reports,
        .fs = fs_virtual("c89-out")
    };

    c89_emit(config, ssa);
    CHECK_REPORTS(reports, "failed to emit ssa");

    fs_copy(config.fs, fs_physical("c89-out"));
}
