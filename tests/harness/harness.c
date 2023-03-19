#include "cthulhu/emit/c89.h"
#include "cthulhu/interface/interface.h"
#include "argparse/argparse.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"
#include "base/util.h"

#include "cthulhu/ssa/ssa.h"
#include "report/report.h"
#include "std/str.h"

#include "io/io.h"

// just kill me already
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

#define CHECK_REPORTS(msg) \
    do { \
        status_t err = end_reports(result.reports, msg, result.reportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

int main(int argc, const char **argv)
{
    common_init();

    driver_t driver = get_driver();
    verbose = true;

    argparse_config_t argparseConfig = {
        .argv = argv,
        .argc = argc,

        .description = format("%s command line interface", driver.name),
        .version = driver.version,

        .reports = begin_reports(),

        .groups = vector_new(0)
    };

    argparse_t result = parse_args(&argparseConfig);
    if (should_exit(&result)) 
    {
        return result.exitCode;
    }

    report_config_t reportConfig = DEFAULT_REPORT_CONFIG;

    size_t totalFiles = vector_len(result.files);
    vector_t *sources = vector_of(totalFiles);
    for (size_t i = 0; i < totalFiles; i++)
    {
        const char *file = vector_get(result.files, i);
        io_t *source = io_file(file, eFileRead | eFileText);
        vector_set(sources, i, source);
    }

    if (totalFiles == 0)
    {
        report(result.reports, eFatal, NULL, "no input files");
        return end_reports(result.reports, "parsing arguments", reportConfig);
    }

    config_t config = {
        .reportConfig = reportConfig,
    };

    cthulhu_t *cthulhu = cthulhu_new(driver, sources, config);

    cthulhu_step_t steps[] = {
        cthulhu_init, 
        cthulhu_parse, 
        cthulhu_forward, 
        cthulhu_resolve, 
        cthulhu_compile,
    };

    size_t totalSteps = sizeof(steps) / sizeof(cthulhu_step_t);

    for (size_t i = 0; i < totalSteps; i++)
    {
        int status = steps[i](cthulhu);
        if (status != EXIT_OK)
        {
            return status;
        }
    }

    // test ssa output

    vector_t *modules = cthulhu_get_modules(cthulhu);
    status_t err = EXIT_OK;

    ssa_module_t *mod = ssa_gen_module(result.reports, modules);
    CHECK_REPORTS("emitting ssa");

    ssa_opt_module(result.reports, mod);
    CHECK_REPORTS("optimizing ssa");

    ssa_emit_module(result.reports, mod);
    CHECK_REPORTS("emitting ssa");

    // test c89 output

    const char *path = vector_get(result.files, 0);
#if OS_WINDOWS
#   define CWD ".\\"
    vector_t *parts = str_split(path, ":");
    CTASSERT(vector_len(parts) == 2);
    const char *name = str_replace(vector_get(parts, 1), "/", ".");
#else
#   define CWD "./"
    const char *name = str_replace(str_lower(path), "/", ".");
#endif

    const char *dir = format(CWD "test-out" NATIVE_PATH_SEPARATOR "%s", name + 1);
    const char *lib = format("%s" NATIVE_PATH_SEPARATOR "it.o", dir);
    const char *src = format("%s" NATIVE_PATH_SEPARATOR "main.c", dir);

#if OS_WINDOWS
    system(format("if not exist \"%s\" md %s", dir, dir));
#else
    system(format("mkdir -p %s", dir));
#endif

    io_t *c89Out = io_file(src, eFileText | eFileWrite);

    CTASSERTF(io_error(c89Out) == 0, "failed to open file: %s", error_string(io_error(c89Out)));

    c89_emit_ssa_modules(result.reports, mod, c89Out);

    err = end_reports(result.reports, "emitting c89", result.reportConfig);
    if (err != EXIT_OK) { return err; }

    io_close(c89Out);

#if OS_WINDOWS
    int status = system(format("cl /nologo /c %s /Fo%s.obj", src, name + 1));
#else
    int status = system(format("cc %s -o %s", src, lib));
#endif

    if (WEXITSTATUS(status) != EXIT_OK)
    {
        report(result.reports, eFatal, NULL, "compilation failed");
        return WEXITSTATUS(status);
    }

    return 0;
}
