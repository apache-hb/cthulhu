#include "base/util.h"
#include "cthulhu/emit/c89.h"
#include "cthulhu/interface/interface.h"
#include "argparse/argparse.h"

#include "base/macros.h"
#include "base/memory.h"

#include "cthulhu/ssa/ssa.h"
#include "report/report.h"
#include "std/str.h"

#include "io/io.h"

// just kill me already
#define MICROSOFT_WINDOWS_WINBASE_H_DEFINE_INTERLOCKED_CPLUSPLUS_OVERLOADS 0

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

static const char *kPostRunNames[] = {"-T", "--then"};
#define TOTAL_POST_RUN_NAMES (sizeof(kPostRunNames) / sizeof(const char *))

int main(int argc, const char **argv)
{
    common_init();

    driver_t driver = get_driver();
    verbose = true;
    
    param_t *postRunParam = string_param("post run command", kPostRunNames, TOTAL_POST_RUN_NAMES);

    group_t *testGroup = group_new("test", "test options", vector_init(postRunParam));

    argparse_config_t argparseConfig = {
        .argv = argv,
        .argc = argc,

        .description = format("%s command line interface", driver.name),
        .version = driver.version,

        .reports = begin_reports(),

        .groups = vector_init(testGroup),
    };

    argparse_t result = parse_args(&argparseConfig);
    if (should_exit(&result)) 
    {
        return result.exitCode;
    }

    report_config_t reportConfig = DEFAULT_REPORT_CONFIG;

    // const char *postRun = get_string_arg(&result, postRunParam, NULL);

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
        return EXIT_ERROR;
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

    module_t *mod = emit_module(result.reports, modules);
    err = end_reports(result.reports, "emitting ssa", result.reportConfig);
    if (err != EXIT_OK) { return err; }

    eval_module(result.reports, mod);
    err = end_reports(result.reports, "evaluating ssa", result.reportConfig);
    if (err != EXIT_OK) { return err; }

    // test c89 output

    make_directory("test-c89");

    const char *path = vector_get(result.files, 0);
    size_t len = strlen(path);
    size_t needed = MIN(strlen(path), 8);
    char *id = ctu_strdup(path + len - needed);
    char *name = format("test-c89/%s.c", str_replace(id, "/", "."));
    io_t *c89Out = io_file(name, eFileText | eFileWrite);
    c89_emit_modules(result.reports, modules, c89Out);

    err = end_reports(result.reports, "emitting c89", result.reportConfig);
    if (err != EXIT_OK) { return err; }

    io_close(c89Out);

    int status = system(format("cl /nologo /c %s /Fo%s.obj", name, name));
    if (status != EXIT_OK)
    {
        report(result.reports, eFatal, NULL, "compilation failed");
        return status;
    }

    DeleteFileA(name);
    DeleteFileA(format("%s.obj", name));

    return 0;
}
