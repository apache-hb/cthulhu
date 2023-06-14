#include "cthulhu/mediator/interface.h"
#include "support/langs.h"

#include "report/report.h"

#include "std/str.h"

#include "io/io.h"

#include "argparse/argparse.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/c89.h"

#include <stdio.h>

#define CHECK_REPORTS(reports, msg) \
    do { \
        int err = end_reports(reports, msg, kReportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

static const report_config_t kReportConfig = {
    .limit = SIZE_MAX,
    .warningsAreErrors = false
};

static const version_info_t kVersion = {
    .license = "GPLv3",
    .desc = "Example compiler interface",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1)
};

static io_t *make_file(reports_t *reports, const char *path, file_flags_t flags)
{
    io_t *io = io_file(path, flags);
    if (io_error(io) != 0)
    {
        message_t *id = report(reports, eFatal, NULL, "failed to open `%s`", path);
        report_note(id, "%s", error_string(io_error(io)));
        return NULL;
    }

    return io;
}

int main(int argc, const char **argv)
{
    verbose = true;

    mediator_t *mediator = mediator_new("example", kVersion);
    lifetime_t *lifetime = lifetime_new(mediator);
    ap_t *ap = ap_new("example", NEW_VERSION(1, 0, 0));
    langs_t langs = get_langs();

    reports_t *reports = lifetime_get_reports(lifetime);

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_add_language(lifetime, ap, lang);
    }

    CHECK_REPORTS(reports, "adding languages");

    for (int i = 1; i < argc; i++)
    {
        const char *path = argv[i];
        const char *ext = str_ext(path);
        const language_t *lang = lifetime_get_language(lifetime, ext);
        
        if (lang == NULL)
        {
            printf("no language found for file: %s\n", path);
        }

        io_t *io = make_file(reports, path, eFileText | eFileRead);
        if (io != NULL)
        {
            lifetime_parse(lifetime, lang, io);
        }

        CHECK_REPORTS(reports, "parsing source");
    }

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
    CHECK_REPORTS(reports, "optimizing ssa");

    ssa_emit_module(reports, ssa);

    io_t *src = make_file(reports, "out.c", eFileText | eFileWrite);
    io_t *hdr = make_file(reports, "out.h", eFileText | eFileWrite);

    CHECK_REPORTS(reports, "failed to open output files");

    emit_config_t emit = {
        .reports = reports,
        .source = src,
        .header = hdr
    };

    emit_ssa_modules(emit, ssa);
    CHECK_REPORTS(reports, "emitting ssa");
}
