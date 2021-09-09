#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "ctu/lir/sema.h"
#include "ctu/emit/emit.h"
#include "ctu/gen/emit.h"

#include <string.h>
#include <stdlib.h>

static reports_t *errors = NULL;

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

        if (fp == NULL) {
            report2(errors, ERROR, NULL, "failed to open file: %s", arg);
        } else {
            vector_push(&sources, fp);
        }
    } else if (MATCH(arg, "-h", "--help")) {
        print_help(name);
    } else if (MATCH(arg, "-v", "--version")) {
        print_version();
    } else if (MATCH(arg, "-src", "--source")) {
        DRIVER = select_driver(NEXT(index, argc, argv));
        return 2;
    } else {
        report2(errors, WARNING, NULL, "unknown argument %s", arg);
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

    errors = begin_reports();

    parse_args(argc, argv);

    int err = end_reports(errors, SIZE_MAX, "command line parsing");
    if (err > 0) {
        return err;
    }

    size_t len = vector_len(sources);
    vector_t *units = vector_new(len);

    int fails = 0;

    for (size_t i = 0; i < len; i++) {
        file_t *fp = vector_get(sources, i);

        const driver_t *driver = driver_for(fp);
        reports_t *reports = begin_reports();

        if (driver->parse == NULL) {
            report2(errors, ERROR, NULL, "unknown file type: %s", fp->path);
        } else {
            void *node = driver->parse(reports, fp);
            vector_push(&units, unit_new(driver, reports, fp, node));
        }

        err = end_reports(reports, SIZE_MAX, format("compilation of `%s`", fp->path));
        fails = MAX(fails, err);
    }

    if (fails > 0) {
        return fails;
    }

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);
        unit->lir = unit->driver->analyze(unit->reports, unit->root);

        err = end_reports(unit->reports, SIZE_MAX, format("semantic analysis of `%s`", unit->file->path));
        fails = MAX(fails, err);
    }

    if (fails > 0) {
        return fails;
    }

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

    return 0;
}
