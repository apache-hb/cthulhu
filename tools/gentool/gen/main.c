#include "argparse/argparse.h"

#include "std/str.h"

#include "base/macros.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "scan/compile.h"

#include "cthulhu/interface/interface.h"

#include "gen/emit.h"

#include "gen-bison.h"
#include "gen-flex.h"

static const char *kBaseNames[] = { "-bn", "--basename" };
#define TOTAL_BASE_NAMES (sizeof(kBaseNames) / sizeof(const char *))

static file_t check_open(reports_t *reports, const char *path, file_flags_t mode)
{
    cerror_t err = 0;
    file_t result = file_open(path, mode, &err);

    if (err != 0)
    {
        report_errno(reports, format("failed to open '%s'", path), err);
    }

    return result;
}

CT_CALLBACKS(kCallbacks, gen);

int main(int argc, const char **argv)
{
    common_init();

    param_t *baseNameParam = string_param("base output path", kBaseNames, TOTAL_BASE_NAMES);
    group_t *codegenGroup = new_group("codegen", "code generation options", vector_init(baseNameParam));

    reports_t *reports = begin_reports();

    argparse_config_t argparseConfig = {
        .argv = argv,
        .argc = argc,

        .description = "gentool cli",
        .version = NEW_VERSION(1, 0, 0),

        .reports = reports,

        .groups = vector_init(codegenGroup),
    };

    argparse_t result = parse_args(&argparseConfig);

    if (should_exit(&result))
    {
        return result.exitCode;
    }

    report_config_t newConfig = {
        .limit = result.reportLimit,
        .warningsAreErrors = result.warningsAsErrors
    };

    size_t totalFiles = vector_len(result.files);
    if (totalFiles != 1)
    {
        report(reports, ERROR, node_invalid(), "only one input may be specified");
        return end_reports(reports, "output validation", newConfig);
    }

    const char *file = vector_get(result.files, 0);
    file_t fd = check_open(reports, file, FILE_READ | FILE_TEXT);
    
    int status = end_reports(reports, "opening source", newConfig);
    if (status != EXIT_OK)
    {
        return status;
    }

    scan_t scan = scan_file(reports, "gen-tool", fd);
    ast_t *data = compile_scanner(scan, &kCallbacks);

    const char *path = get_string(&result, baseNameParam, "gen");

    cerror_t err = make_directory(path);
    if (err != 0)
    {
        report_errno(reports, format("failed to create directory '%s'", path), err);
    }

    file_t header = check_open(reports, format("%s" NATIVE_PATH_SEPARATOR "ast.gen.h", path), FILE_WRITE | FILE_TEXT);
    file_t source = check_open(reports, format("%s" NATIVE_PATH_SEPARATOR "ast.gen.c", path), FILE_WRITE | FILE_TEXT);

    status = end_reports(reports, "opening outputs", newConfig);
    if (status != EXIT_OK)
    {
        return status;
    }

    emit_t config = {
        .reports = reports,
        .root = data,
        .path = path,
        .header = header,
        .source = source
    };

    emit(&config);

    return end_reports(reports, "writing outputs", newConfig);
}
