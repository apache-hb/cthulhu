#include "cthulhu/mediator/interface.h"
#include "support/langs.h"

#include "report/report.h"

#include "std/str.h"

#include "io/io.h"

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

int main(int argc, const char **argv)
{
    verbose = true;

    mediator_t *mediator = mediator_new("example", kVersion);
    lifetime_t *lifetime = lifetime_new(mediator);
    langs_t langs = get_langs();

    reports_t *reports = lifetime_get_reports(lifetime);

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

        io_t *io = io_file(path, eFileRead | eFileText);
        io_error_t err = io_error(io);
        if (err != 0)
        {
            printf("failed to load source `%s`%s", path, error_string(err));
            continue;
        }

        lifetime_parse(lifetime, lang, io);

        CHECK_REPORTS(reports, "parsing source");
    }

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_REPORTS(reports, msg);
    }
}
