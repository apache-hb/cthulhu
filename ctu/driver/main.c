#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "ctu/frontend/pl0/driver.h"

#include <string.h>

static const char *name = NULL;

static vector_t *sources = NULL;

static int parse_arg(int index, char **argv) {
    char *arg = argv[index];
    
    if (!startswith(arg, "--")) {
        vector_push(&sources, arg);
    } else {
        report(ERROR, "unknown argument %s", arg);
    }

    return 1;
}

static void parse_args(int argc, char **argv) {
    sources = vector_new(4);

    for (int i = 1; i < argc;) {
        i += parse_arg(i, argv);
    }

    end_report("commandline parsing");
}

int main(int argc, char **argv) {
    name = argv[0];

    begin_report(20);

    parse_args(argc, argv);

    pl0_driver(sources);

    end_report("PL/0 compilation");

    if (argc == 1) {
        return 0;
    }

    return 0;
}
