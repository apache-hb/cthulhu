#include "ast.h"
#include "emit.h"

#include "cthulhu/interface/interface.h"
#include "cthulhu/ast/compile.h"
#include "platform/file.h"
#include "cthulhu/interface/runtime.h"

#include "std/str.h"

#include "gen-bison.h"
#include "gen-flex.h"

CT_CALLBACKS(kCallbacks, gen);

int main(int argc, const char **argv)
{
    common_init();

    // TODO: extract command line parser out and use that here
    // gentool <input> <output> [--compiler | --tooling]
    if (argc < 4)
    {
        printf("Usage: gentool <input> <output> [--compiler | --tooling]\n");
        return 1;
    }

    const char *input = argv[1];
    const char *output = argv[2];
    const char *mode = argv[3];

    report_config_t reportConfig = {
        .limit = 100,
        .warningsAreErrors = false,
    };

    reports_t *reports = begin_reports();

    cerror_t err = 0;
    file_t file = file_open(input, FILE_READ | FILE_TEXT, &err);
    scan_t scan = scan_file(reports, "gentool", file);
    ast_t *ast = compile_string(&scan, &kCallbacks);

    int status = end_reports(reports, "compiling", reportConfig);
    if (status != EXIT_OK)
    {
        return status;
    }

    map_t *result = NULL;

    if (str_equal(mode, "--compiler"))
    {
        result = emit_compiler(reports, ast);
    }
    else if (str_equal(mode, "--tooling"))
    {
        result = emit_tooling(reports, ast);
    }
    else
    {
        printf("unknown mode: %s\n", mode);
        return 1;
    }

    status = end_reports(reports, "emitting", reportConfig);
    if (status != EXIT_OK)
    {
        return status;
    }

    map_iter_t iter = map_iter(result);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        const char *ext = entry.key;
        const stream_t *data = entry.value;

        char *path = format("%s-%s", output, ext);
        file_t file = file_open(path, FILE_WRITE | FILE_TEXT, &err);

        file_write(file, stream_data(data), stream_len(data), &err);
    }

    return end_reports(reports, "writing", reportConfig);
}

void generror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}
