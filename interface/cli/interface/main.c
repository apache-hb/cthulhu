#include "argparse/argparse.h"

#include "std/str.h"

#include "cthulhu/interface/interface.h"

#include "cthulhu/emit/c89.h"

static const char *kOutputFileNames[] = { "-o", "--ouput" };
#define TOTAL_OUTPUT_FILE_NAMES (sizeof(kOutputFileNames) / sizeof(const char *))

int main(int argc, const char **argv)
{
    common_init();

    driver_t driver = get_driver();

    param_t *outputFileNameParam = new_param(PARAM_STRING, "output file name", kOutputFileNames, TOTAL_OUTPUT_FILE_NAMES);
    group_t *codegenGroup = new_group("codegen", "code generation options", vector_init(outputFileNameParam));

    reports_t *reports = begin_reports();

    report_config_t reportConfig = DEFAULT_REPORT_CONFIG;

    arg_parse_config_t argparseConfig = {
        .argc = argc,
        .argv = argv,

        .description = format("%s command line interface", driver.name),
        .version = driver.version,

        .reports = reports,

        .groups = vector_init(codegenGroup),
    };

    arg_parse_result_t result = arg_parse(&argparseConfig);

    if (result.exitCode != EXIT_OK)
    {
        return result.exitCode;
    }

    const char *outFile = get_string_arg(outputFileNameParam, "out.c");

    size_t totalFiles = vector_len(result.extra);
    vector_t *sources = vector_of(totalFiles);
    for (size_t i = 0; i < totalFiles; i++)
    {
        const char *file = vector_get(result.extra, i);
        source_t *source = source_file(file);
        vector_set(sources, i, source);
    }

    int status = end_reports(reports, "command line parsing", reportConfig);
    if (status != 0)
    {
        return status;
    }

    config_t config = {
        .reportConfig = reportConfig,
    };

    cthulhu_t *cthulhu = cthulhu_new(driver, sources, config);

    cthulhu_step_t steps[] = {
        cthulhu_init,    // init cthulhu instance
        cthulhu_parse,   // parse source files
        cthulhu_forward, // forward declarations
        cthulhu_resolve, // resolve declarations
        cthulhu_compile, // compile to hlir
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

    cerror_t error = 0;
    file_t out = file_open(outFile, FILE_WRITE | FILE_BINARY, &error);

    if (error != 0)
    {
        message_t *id = report(reports, ERROR, node_invalid(), "failed to open file `%s`", outFile);
        report_note(id, "%s", error_string(error));
        return EXIT_ERROR;
    }

    stream_t *stream = c89_emit_modules(reports, allModules);

    file_write(out, stream_data(stream), stream_len(stream), &error);

    file_close(out);

    return end_reports(reports, "emitting code", reportConfig);
}
