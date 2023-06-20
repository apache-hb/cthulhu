#include "cthulhu/mediator/interface.h"
#include "support/langs.h"

#include "report/report.h"

#include "base/panic.h"

#include "std/str.h"
#include "std/vector.h"

#include "io/io.h"
#include "io/fs.h"

#include "argparse/argparse.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/c89.h"

#include <stdio.h>

// just kill me already
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OS_WINDOWS
#   include <windows.h>
#   define CWD ".\\"
#   define OBJ ".obj"
#else
#   include <unistd.h>
#   define CWD "./"
#   define OBJ ".o"
#endif

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
    .desc = "Test harness",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1)
};

static io_t *make_file(const char *path, file_flags_t flags)
{
    io_t *io = io_file(path, flags);
    size_t err = io_error(io);
    if (err != 0)
    {
        CTASSERTF(false, "failed to open `%s`: %s", path, error_string(err));
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
        lifetime_config_language(lifetime, ap, lang);
    }

    for (size_t i = 0; i < langs.size; i++)
    {
        const language_t *lang = langs.langs + i;
        lifetime_add_language(lifetime, lang);
    }

    CHECK_REPORTS(reports, "adding languages");

    // harness.exe <name> [files...]
    CTASSERT(argc > 2);

    for (int i = 2; i < argc; i++)
    {
        const char *path = argv[i];
        const char *ext = str_ext(path);
        const language_t *lang = lifetime_get_language(lifetime, ext);

        io_t *io = make_file(path, eFileText | eFileRead);

        lifetime_parse(lifetime, lang, io);

        CHECK_REPORTS(reports, "parsing source");
    }

    for (size_t stage = 0; stage < eStageTotal; stage++)
    {
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_REPORTS(reports, msg);
    }

    lifetime_check(lifetime);
    CHECK_REPORTS(reports, "validations failed");

    vector_t *mods = lifetime_get_modules(lifetime);
    ssa_module_t *ssa = ssa_gen_module(reports, mods);
    CHECK_REPORTS(reports, "generating ssa");

    ssa_opt_module(reports, ssa);
    CHECK_REPORTS(reports, "optimizing ssa");

    ssa_emit_module(reports, ssa);

    const char *path = argv[1];

    const char *dir = format(CWD "test-out" NATIVE_PATH_SEPARATOR "%s", path);
    const char *libPath = format("%s" NATIVE_PATH_SEPARATOR "it" OBJ, dir);
    const char *srcPath = format("%s" NATIVE_PATH_SEPARATOR "out.c", dir);
    const char *hdrPath = format("%s" NATIVE_PATH_SEPARATOR "out.h", dir);

#if OS_WINDOWS
    system(format("if not exist \"%s\" md %s", dir, dir));
#else
    system(format("mkdir -p %s", dir));
#endif

    CHECK_REPORTS(reports, "failed to open output files");

    c89_emit_t emit = {
        .reports = reports,
        .fs = fs_virtual("c89")
    };

    c89_emit(emit, ssa);
    CHECK_REPORTS(reports, "emitting ssa");

#if OS_WINDOWS
    int status = system(format("cl /nologo /c %s /Fo%s", srcPath, libPath));
    if (status == -1)
    {
        report(reports, eFatal, NULL, "compilation failed %d", errno);
    }
#else
    int status = system(format("cc %s -c -o%s", srcPath, libPath));
    if (WEXITSTATUS(status) != EXIT_OK)
    {
        report(reports, eFatal, NULL, "compilation failed %d", WEXITSTATUS(status));
    }
#endif

    CHECK_REPORTS(reports, "compiling");
}
