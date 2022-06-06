#include "argparse/argparse.h"

#include "std/str.h"

#include "cthulhu/interface/interface.h"

static group_t *codegen_group(void)
{
    vector_t *params = vector_new(32);
    ADD_FLAG(params, PARAM_STRING, "output file name", { "-o", "--output" });
    return new_group("codegen", "code generation options", params);
}

static vector_t *build_groups(void)
{
    vector_t *result = vector_new(64);
    vector_push(&result, codegen_group());
    return result;
}

int main(int argc, const char **argv)
{
    common_init();

    driver_t driver = get_driver();

    reports_t *reports = begin_reports();

    arg_parse_config_t config = {
        .argc = argc,
        .argv = argv,

        .description = format("%s command line interface", driver.name),
        .version = driver.version,

        .reports = reports,

        .groups = build_groups(),
    };

    arg_parse_result_t result = arg_parse(&config);

    if (result.exitCode != EXIT_OK)
    {
        return result.exitCode;
    }

    const char *outFile = get_string_arg(result, )

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
