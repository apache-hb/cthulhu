#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "ctu/frontend/pl0/driver.h"

#include <string.h>
#include <stdlib.h>

#include "cmd.c"

static const char *name = NULL;

/* vector_t<file_t*> */
static vector_t *sources = NULL;

#define MATCH(arg, a, b) (startswith(arg, a) || startswith(arg, b))

static int parse_arg(int index, char **argv) {
    const char *arg = argv[index];
    
    if (!startswith(arg, "--")) {
        file_t *fp = ctu_open(arg, "rb");
        vector_push(&sources, fp);
    } else if (MATCH(arg, "-h", "--help")) {
        print_help(name);
    } else if (MATCH(arg, "-v", "--version")) {
        print_version();
    } else {
        report(ERROR, "unknown argument %s", arg);
    }

    return 1;
}

static void parse_args(int argc, char **argv) {
    if (argc == 1) {
        print_help(name);
    }

    sources = vector_new(4);

    for (int i = 1; i < argc;) {
        i += parse_arg(i, argv);
    }

    end_report("commandline parsing");
}

int main(int argc, char **argv) {
    name = argv[0];
    init_memory();

    begin_report(20);

    parse_args(argc, argv);

    pl0_driver(sources);

    end_report("PL/0 compilation");

    for (size_t i = 0; i < vector_len(sources); i++) {
        file_t *fp = vector_get(sources, i);
        ctu_close(fp);
    }

    return 0;
}
