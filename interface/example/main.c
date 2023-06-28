#include "cthulhu/mediator/interface.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/emit.h"

#include "support/langs.h"

#include "report/report.h"

#include "std/str.h"
#include "std/map.h"

#include "io/io.h"
#include "io/fs.h"

#include "argparse/argparse.h"

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

static io_t *make_file(reports_t *reports, const char *path, os_access_t flags)
{
    io_t *io = io_file(path, flags);
    if (io_error(io) != 0)
    {
        message_t *id = report(reports, eFatal, NULL, "failed to open `%s`", path);
        report_note(id, "%s", os_decode(io_error(io)));
        return NULL;
    }

    return io;
}

int main(int argc, const char **argv)
{
    verbose = true;

    mediator_t *mediator = mediator_new("example", kVersion);
    lifetime_t *lifetime = lifetime_new(mediator);
    reports_t *reports = lifetime_get_reports(lifetime);

    ap_t *ap = ap_new("example", NEW_VERSION(1, 0, 0));
    langs_t langs = get_langs();

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_config_language(lifetime, ap, lang);
    }

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_add_language(lifetime, lang);
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

        io_t *io = make_file(reports, path, eAccessRead | eAccessText);
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

    map_t *modmap = lifetime_get_modules(lifetime);

    ssa_module_t *ssa = ssa_compile(modmap);
    CHECK_REPORTS(reports, "generating ssa");

    fs_t *fs = fs_virtual(reports, "out");

    emit_c89(reports, fs, ssa);
    CHECK_REPORTS(reports, "emitting ssa");

    fs_t *out = fs_physical(reports, "out");
    CHECK_REPORTS(reports, "creating output directory");

    fs_sync(out, fs);
    CHECK_REPORTS(reports, "syncing output directory");
}
