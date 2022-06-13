#include "argparse/argparse.h"

#include "std/str.h"

#include "base/macros.h"

#include "report/report.h"

#include "scan/compile.h"

#include "cthulhu/interface/interface.h"

#include "gen-bison.h"
#include "gen-flex.h"

static const char *kBaseNames[] = { "-bn", "--basename" };
#define TOTAL_BASE_NAMES (sizeof(kBaseNames) / sizeof(const char *))

CT_CALLBACKS(kCallbacks, gen);

int main(int argc, const char **argv)
{
    common_init();

    param_t *baseNameParam = string_param("base output name", kBaseNames, TOTAL_BASE_NAMES);
    group_t *codegenGroup = new_group("codegen", "code generation options", vector_init(baseNameParam));

    reports_t *reports = begin_reports();

    report_config_t reportConfig = DEFAULT_REPORT_CONFIG;

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

    const char *baseName = get_string(&result, baseNameParam, "out");

    size_t totalFiles = vector_len(result.files);
    if (totalFiles != 1)
    {
        report(reports, ERROR, node_invalid(), "only one input may be specified");
        return end_reports(reports, "output validation", reportConfig);
    }

    const char *file = vector_get(result.files, 0);
    cerror_t err = 0;
    file_t fd = file_open(file, FILE_READ | FILE_TEXT, &err);
    
    scan_t scan = scan_file(reports, "gen-tool", fd);
    void *data = compile_scanner(scan, &kCallbacks);
}
