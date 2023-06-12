#include "cthulhu/mediator/interface.h"
#include "support/langs.h"

#include "report/report.h"

#include "std/str.h"

#include "io/io.h"

#include <stdio.h>

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
    reports_t *reports = begin_reports();
    langs_t langs = get_langs();

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_add_language(lifetime, lang);
    }

    for (int i = 1; i < argc; i++)
    {
        const char *path = argv[i];
        const char *ext = str_ext(path);
        const language_t *lang = lifetime_get_language(lifetime, ext);
        
        logverbose("path: %s", path);

        if (lang == NULL)
        {
            printf("no language found for file: %s\n", path);
        }

        io_t *io = io_file(path, eFileRead | eFileText);
        io_error_t err = io_error(io);
        if (err != 0)
        {
            logverbose("failed to load source `%s`%s", path, error_string(err));
            continue;
        }

        lifetime_parse(lifetime, reports, lang, io);
    }
}
