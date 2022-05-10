#include "cthulhu/interface/runtime.h"
#include "cmd.h"
#include "cthulhu/hlir/init.h"
#include "cthulhu/util/version-def.h"

#include "cmd.h"
#include "cthulhu/ast/compile.h"
#include "cthulhu/emit/c89.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/vector.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static void print_version(driver_t driver)
{
    int major = VERSION_MAJOR(driver.version);
    int minor = VERSION_MINOR(driver.version);
    int patch = VERSION_PATCH(driver.version);
    printf("%s: %d.%d.%d\n", driver.name, major, minor, patch);
}

static char *join_names(const char **names, size_t num)
{
    vector_t *vec = vector_of(num);

    for (size_t i = 0; i < num; i++)
    {
        vector_set(vec, i, (char *)names[i]);
    }

    return str_join(", ", vec);
}

static void print_help(const char **argv)
{
    printf("usage: %s <files... & objects... & options...>\n", argv[0]);

#define SECTION(id) printf("%s options:\n", id);
#define COMMAND(name, type, initial, description, ...)                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        const char *names[] = __VA_ARGS__;                                                                             \
        size_t total = sizeof(names) / sizeof(const char *);                                                           \
        const char *parts = join_names(names, total);                                                                  \
        printf("  %-20s : %s\n", parts, description);                                                                  \
    } while (0);

#include "flags.inc"
}

#define END_STAGE(name)                                                                                                \
    do                                                                                                                 \
    {                                                                                                                  \
        status = end_reports(reports, name, &reportSettings);                                                          \
        if (status != 0)                                                                                               \
        {                                                                                                              \
            return status;                                                                                             \
        }                                                                                                              \
    } while (0)

int main(int argc, const char **argv)
{
    common_init();

    driver_t driver = get_driver();

    reports_t *reports = begin_reports();
    commands_t commands = {0};
    int status = parse_commandline(reports, &commands, argc, argv);
    if (status != 0)
    {
        return status;
    }

    report_config_t reportSettings = {.limit = commands.warningLimit,
                                            .warningsAreErrors = commands.warningsAsErrors};

    verbose = commands.verboseLogging;
    logverbose("setup verbose logging");

    vector_t *files = commands.files;
    const char *outFile = commands.outputFile == NULL ? "out.c" : commands.outputFile;

    if (commands.printHelp)
    {
        print_help(argv);
        return 0;
    }

    if (commands.printVersion)
    {
        print_version(driver);
        return 0;
    }

    END_STAGE("commandline");

    report_config_t reportConfig = {
        .limit = commands.warningLimit,
        .warningsAreErrors = commands.warningsAsErrors,
    };

    config_t config = {
        .reportConfig = reportConfig
    };

    cthulhu_t *cthulhu = cthulhu_new(driver, files, config);

    int(*steps[])(cthulhu_t*) = {
        cthulhu_init,
        cthulhu_parse,
        cthulhu_forward,
        cthulhu_resolve,
        cthulhu_compile
    };

    size_t totalSteps = sizeof(steps) / sizeof(cthulhu_step_t);

    for (size_t i = 0; i < totalSteps; i++)
    {
        status = steps[i](cthulhu);
        if (status != 0)
        {
            return status;
        }
    }

    vector_t *allModules = cthulhu_get_modules(cthulhu);

    error_t error = 0;
    file_t out = file_open(outFile, FILE_WRITE | FILE_BINARY, &error);

    if (error != 0)
    {
        message_t *id = report(reports, ERROR, NULL, "failed to open file `%s`", outFile);
        report_note(id, "%s", error_string(error));
        return status;
    }

    c89_emit_modules(reports, allModules, out);

    file_close(out);

    logverbose("finished compiling %zu modules", vector_len(allModules));

    return end_reports(reports, "emitting code", &reportSettings);
}
