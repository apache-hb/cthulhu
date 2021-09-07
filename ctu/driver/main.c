#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "ctu/lir/sema.h"
#include "ctu/emit/emit.h"
#include "ctu/gen/emit.h"

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
        file_t *fp = ctu_open(reports, arg, "rb");
        vector_push(&sources, fp);
    } else if (MATCH(arg, "-h", "--help")) {
        print_help(name);
    } else if (MATCH(arg, "-v", "--version")) {
        print_version();
    } else if (MATCH(arg, "-src", "--source")) {
        DRIVER = select_driver(NEXT(index, argc, argv));
        return 2;
    } else {
        reportf(reports, WARNING, NULL, "unknown argument %s", arg);
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

    int error = end_report(reports, "commandline parsing");
    if (error != 0) {
        exit(error);
    }
}

typedef struct {
    const driver_t *driver;
    reports_t *reports;
    file_t *file;
    void *root;
    lir_t *lir;
} unit_t;

static unit_t *unit_new(const driver_t *driver, reports_t *reports, file_t *file, void *node) {
    unit_t *unit = ctu_malloc(sizeof(unit_t));
    unit->driver = driver;
    unit->reports = reports;
    unit->file = file;
    unit->root = node;
    return unit;
}

int main(int argc, char **argv) {
    name = argv[0];
    init_memory();

    reports = begin_report(20);

    parse_args(argc, argv);

    size_t len = vector_len(sources);
    vector_t *units = vector_new(len);

    for (size_t i = 0; i < len; i++) {
        file_t *fp = vector_get(sources, i);

        const driver_t *driver = driver_for(fp);

        if (driver->parse == NULL) {
            reportf(reports, ERROR, NULL, "unknown file type: %s", fp->path);
        } else {
            reports_t *errs = begin_report(20);
            void *data = driver->parse(errs, fp);
            vector_push(&units, unit_new(driver, errs, fp, data));
        }

        end_report(reports, format("compilation of `%s`", fp->path));
    }

    int error = end_report(reports, "compilation");
    if (error != 0) {
        exit(error);
    }

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);
        unit->lir = unit->driver->analyze(unit->reports, unit->root);

        end_report(unit->reports, format("semantic analysis of `%s`", unit->file->path));
    }

    end_report(true, "semantic analysis");

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);
        lir_t *lir = unit->lir;
        printf("%s\n", print_lir(lir));
        /*
        emit_c(stdout, unit->lir);
        */
       
        module_t *mod = module_build(unit->lir);
        module_print(stdout, mod);
    }

    end_report(true, "code generation");

    return 0;
}
