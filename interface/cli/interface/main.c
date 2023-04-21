#include "argparse/argparse.h"

#include "std/str.h"

#include "base/macros.h"
#include "base/memory.h"

#include "cthulhu/interface/interface.h"

#include "cthulhu/ssa/ssa.h"

#include "cthulhu/emit/c89.h"

#include "io/io.h"

#include <stdio.h>

static const char *kOutputFileNames[] = {"-o", "--output"};
#define TOTAL_OUTPUT_FILE_NAMES (sizeof(kOutputFileNames) / sizeof(const char *))

static const char *kHlirNames[] = {"--enable-hlir", "-hlir"};
#define TOTAL_HLIR_NAMES (sizeof(kHlirNames) / sizeof(const char *))

static const char *kHeaderNames[] = {"--enable-header", "-header"};
#define TOTAL_HEADER_NAMES (sizeof(kHeaderNames) / sizeof(const char *))

#define CHECK_REPORTS(msg) \
    do { \
        status_t err = end_reports(reports, msg, reportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

static bool check_io(reports_t *reports, io_t *io)
{
    if (io == NULL) 
    {
        return true;
    }

    if (io_error(io) != 0)
    {
        message_t *id = report(reports, eFatal, node_invalid(), "failed to open file `%s`", io_name(io));
        report_note(id, "%s", error_string(io_error(io)));
        return false;
    }

    return true;
}

int main(int argc, const char **argv)
{
    common_init();

    // TODO: dynamically link to the language driver
    // this should be an option at build time so we
    // only need to build one of each interface
    // rather than one of each interface for each driver
    driver_t driver = get_driver();

    param_t *outputFileNameParam = string_param("output file name", kOutputFileNames, TOTAL_OUTPUT_FILE_NAMES);
    param_t *enableHlirParam = bool_param("use hlir codegen instead of ssa (deprecated)", kHlirNames, TOTAL_HLIR_NAMES);
    param_t *headerNameParam = string_param("c89 header file name (default does not generate a header)", kHeaderNames, TOTAL_HEADER_NAMES);

    vector_t *codegenParams = vector_new(3);
    vector_push(&codegenParams, outputFileNameParam);
    vector_push(&codegenParams, enableHlirParam);
    vector_push(&codegenParams, headerNameParam);

    group_t *codegenGroup = group_new("codegen", "code generation options", codegenParams);

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

    const char *outFile = get_string_arg(&result, outputFileNameParam, "out");
    const char *headerFile = get_string_arg(&result, headerNameParam, NULL);
    bool enableHlir = get_bool_arg(&result, enableHlirParam, false);

    size_t totalFiles = vector_len(result.files);
    vector_t *sources = vector_of(totalFiles);
    for (size_t i = 0; i < totalFiles; i++)
    {
        const char *file = vector_get(result.files, i);
        io_t *source = io_file(file, eFileRead | eFileText);
        vector_set(sources, i, source);
    }

    CHECK_REPORTS("command line parsing");

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

    status_t status = EXIT_OK;
    for (size_t i = 0; i < totalSteps; i++)
    {
        status = steps[i](cthulhu);
        if (status != EXIT_OK)
        {
            return status;
        }
    }

    vector_t *allModules = cthulhu_get_modules(cthulhu);
    io_t *outSource = io_file(format("%s.c", outFile), eFileWrite | eFileBinary);
    
    io_t *outHeader = headerFile != NULL 
        ? io_file(format("%s.h", headerFile), eFileWrite | eFileBinary)
        : NULL;

    if (!check_io(reports, outSource)) 
    {
        return end_reports(reports, "opening file", reportConfig);
    }

    if (!check_io(reports, outHeader)) 
    {
        return end_reports(reports, "opening file", reportConfig);
    }

    emit_config_t emitConfig = {
        .reports = reports,
        .source = outSource,
        .header = outHeader
    };

    if (!enableHlir)
    {
        ssa_module_t *mod = ssa_gen_module(reports, allModules);
        CHECK_REPORTS("generating ssa");

        ssa_opt_module(reports, mod);
        CHECK_REPORTS("optimizing ssa");

        ssa_emit_module(reports, mod);
        CHECK_REPORTS("emitting ssa");

        c89_emit_ssa_modules(emitConfig, mod);

        return end_reports(reports, "generating c89 from ssa", reportConfig);
    }

    c89_emit_hlir_modules(emitConfig, allModules);

    return end_reports(reports, "emitting code", reportConfig);
}
