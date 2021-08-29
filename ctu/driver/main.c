#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "ctu/sema/sema.h"
#include "ctu/emit/emit.h"

#include <string.h>
#include <stdlib.h>

#include "cmd.c"

static const char *name = NULL;

/* vector_t<file_t*> */
static vector_t *sources = NULL;

#define MATCH(arg, a, b) (startswith(arg, a) || startswith(arg, b))
#define NEXT(idx, argc, argv) (idx + 1 >= argc ? NULL : argv[idx + 1])

static int parse_arg(int index, int argc, char **argv) {
    const char *arg = argv[index];
    
    if (!startswith(arg, "-")) {
        file_t *fp = ctu_open(arg, "rb");
        vector_push(&sources, fp);
    } else if (MATCH(arg, "-h", "--help")) {
        print_help(name);
    } else if (MATCH(arg, "-v", "--version")) {
        print_version();
    } else if (MATCH(arg, "-src", "--source")) {
        DRIVER = select_driver(NEXT(index, argc, argv));
        return 2;
    } else {
        report(WARNING, "unknown argument %s", arg);
    }

    return 1;
}

static void parse_args(int argc, char **argv) {
    if (argc == 1) {
        print_help(name);
    }

    sources = vector_new(4);

    for (int i = 1; i < argc;) {
        i += parse_arg(i, argc, argv);
    }

    end_report(true, "commandline parsing");
}

int main(int argc, char **argv) {
    name = argv[0];
    init_memory();

    begin_report(20);

    parse_args(argc, argv);

    size_t len = vector_len(sources);
    vector_t *nodes = vector_new(len);

    for (size_t i = 0; i < len; i++) {
        file_t *fp = vector_get(sources, i);

        const driver_t *driver = driver_for(fp);

        if (driver->driver == NULL) {
            report(ERROR, "unknown file type: %s", fp->path);
        } else {
            node_t *node = driver->driver(fp);
            vector_push(&nodes, node);
        }

        end_report(false, format("compilation of `%s`", fp->path));

        ctu_close(fp);
    }

    end_report(true, "compilation");

    size_t total = vector_len(nodes);

    for (size_t i = 0; i < total; i++) {
        node_t *node = vector_get(nodes, i);
        lir_t *lir = sema_module(node);

        vector_set(nodes, i, lir);

        end_report(false, format("semantic analysis of `%s`", node->scan->path));
    }

    end_report(true, "semantic analysis");

    for (size_t i = 0; i < total; i++) {
        lir_t *lir = vector_get(nodes, i);

        emit_c(stdout, lir);
    }

    return 0;
}
