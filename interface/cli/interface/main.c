#include "argparse/argparse.h"

#include "std/str.h"

#include "base/macros.h"

#include "cthulhu/interface/interface.h"

#include "cthulhu/emit/c89.h"
#include "cthulhu/ssa/ssa.h"

static const char *kOutputFileNames[] = {"-o", "--output"};
#define TOTAL_OUTPUT_FILE_NAMES (sizeof(kOutputFileNames) / sizeof(const char *))

static const char *kSSANames[] = {"--enable-ssa", "-ssa"};
#define TOTAL_SSA_NAMES (sizeof(kSSANames) / sizeof(const char *))

int main(int argc, const char **argv)
{
    common_init();

    // TODO: dynamically link to the language driver
    // this should be an option at build time so we
    // only need to build one of each interface
    // rather than one of each interface for each driver
    driver_t driver = get_driver();

    param_t *outputFileNameParam = string_param("output file name", kOutputFileNames, TOTAL_OUTPUT_FILE_NAMES);
    param_t *enableSSAParam = bool_param("enable ssa codegen (experimental)", kSSANames, TOTAL_SSA_NAMES);

    vector_t *codegenParams = vector_new(2);
    vector_push(&codegenParams, outputFileNameParam);
    vector_push(&codegenParams, enableSSAParam);

    group_t *codegenGroup = new_group("codegen", "code generation options", codegenParams);

    reports_t *reports = begin_reports();

    report_config_t reportConfig = DEFAULT_REPORT_CONFIG;

    argparse_config_t argparseConfig = {
        .argv = argv,
        .argc = argc,

        .description = format("%s command line interface", driver.name),
        .version = driver.version,

        .reports = reports,

        .groups = vector_init(codegenGroup),
    };

    argparse_t result = parse_args(&argparseConfig);

    if (should_exit(&result))
    {
        return result.exitCode;
    }

    const char *outFile = get_string_arg(&result, outputFileNameParam, "out.c");
    bool enableSsa = get_bool_arg(&result, enableSSAParam, false);

    size_t totalFiles = vector_len(result.files);
    vector_t *sources = vector_of(totalFiles);
    for (size_t i = 0; i < totalFiles; i++)
    {
        const char *file = vector_get(result.files, i);
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

    if (enableSsa)
    {
        module_t *mod = ssa_compile(reports, allModules);
        UNUSED(mod);

        status = end_reports(reports, "ssa codegen", reportConfig);
        if (status != EXIT_OK)
        {
            return status;
        }
    }

    cerror_t error = 0;
    file_t out = file_open(outFile, eFileWrite | eFileBinary, &error);

    if (error != 0)
    {
        message_t *id = report(reports, eFatal, node_invalid(), "failed to open file `%s`", outFile);
        report_note(id, "%s", error_string(error));
        return end_reports(reports, "opening file", reportConfig);
    }

    stream_t *stream = c89_emit_modules(reports, allModules);

    file_write(out, stream_data(stream), stream_len(stream), &error);

    file_close(out);

    return end_reports(reports, "emitting code", reportConfig);
}
