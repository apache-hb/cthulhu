#include "cthulhu/mediator/interface.h"
#include "support/langs.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/emit.h"

#include "report/report.h"

#include "base/panic.h"

#include "std/str.h"
#include "std/vector.h"
#include "std/map.h"

#include "io/io.h"
#include "io/fs.h"

#include "argparse/argparse.h"

#include <stdio.h>

// just kill me already
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if OS_WINDOWS
#   include <windows.h>
#   define CWD ".\\"
#else
#   include <unistd.h>
#   define CWD "./"
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

static io_t *make_file(const char *path, os_access_t flags)
{
    io_t *io = io_file(path, flags);
    CTASSERT(io_error(io) == 0);
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

        io_t *io = make_file(path, eAccessRead | eAccessText);

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

    map_t *modmap = lifetime_get_modules(lifetime);

    ssa_result_t ssa = ssa_compile(modmap);
    CHECK_REPORTS(reports, "generating ssa");

    ssa_opt(reports, ssa);
    CHECK_REPORTS(reports, "optimizing ssa");

    fs_t *fs = fs_virtual(reports, "out");

    emit_options_t baseOpts = {
        .reports = reports,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    ssa_emit_options_t emitOpts = {
        .opts = baseOpts
    };

    ssa_emit_result_t ssaResult = emit_ssa(&emitOpts);
    CHECK_REPORTS(reports, "emitting ssa");
    UNUSED(ssaResult); // TODO: check for errors

    c89_emit_options_t c89Opts = {
        .opts = baseOpts
    };

    c89_emit_result_t c89Result = emit_c89(&c89Opts);
    CHECK_REPORTS(reports, "emitting c89");

    OS_RESULT(const char *) cwd = os_dir_current();
    CTASSERT(os_error(cwd) == 0);

    const char *testDir = format("%s" NATIVE_PATH_SEPARATOR "test-out", OS_VALUE(const char*, cwd));
    const char *runDir = format("%s" NATIVE_PATH_SEPARATOR "%s", testDir, argv[1]);

    fs_t *out = fs_physical(reports, runDir);
    CHECK_REPORTS(reports, "creating output directory");

    fs_sync(out, fs);
    CHECK_REPORTS(reports, "syncing output directory");

    size_t len = vector_len(c89Result.sources);
    vector_t *sources = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const char *part = vector_get(c89Result.sources, i);
        char *path = format("%s" NATIVE_PATH_SEPARATOR "%s", runDir, part);
        vector_set(sources, i, path);
    }

#if OS_WINDOWS
    int status = system(format("cl /nologo /c %s /I%s\\include", str_join(" ", sources), runDir));
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

    logverbose("done");
}
