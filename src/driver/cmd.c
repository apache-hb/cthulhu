#include "cmd.h"

#include "cmd-bison.h"
#include "cmd-flex.h"
#include "cthulhu/ast/compile.h"
#include "cthulhu/driver/driver.h"
#include "cthulhu/util/macros.h"
#include "cthulhu/util/str.h"

CT_CALLBACKS(kCallbacks, cmd);

void cmd_add_file(commands_t *commands, char *path) {
    vector_push(&commands->sources, path);
}

void cmderror(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}

static char *join_args(int argc, const char **argv) {
    vector_t *vec = vector_of(argc - 1);
    for (int i = 1; i < argc; i++) {
        vector_set(vec, i - 1, (char*)argv[i]);
    }
    return str_join(" ", vec);
}

int parse_commandline(reports_t *reports, commands_t *commands, int argc, const char **argv) {
    char *args = join_args(argc, argv);

    scan_t scan = scan_string(reports, "command-line", "<command-line>", args);

    int status = end_reports(reports, DEFAULT_REPORT_LIMIT, "command line parsing");
    if (status != 0) { return status; }

    commands->sources = vector_new(8);

    scan_set(&scan, commands);
    compile_string(&scan, &kCallbacks);

    return end_reports(reports, commands->warningLimit, "command line parsing");
}
