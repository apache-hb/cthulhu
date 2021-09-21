#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "driver.h"
#include "cmd.h"
#include "ctu/lir/sema.h"
#include "ctu/emit/emit.h"
#include "ctu/gen/emit.h"

#include "ctu/backend/gcc/gcc.h"
#include "ctu/backend/c89/c89.h"

#include <string.h>
#include <stdlib.h>

static reports_t *errors = NULL;

static const char *name = NULL;

typedef struct {
    const driver_t *driver;
    reports_t *reports;
    file_t *file;
    void *root;
    lir_t *lir;
    module_t *mod;
} unit_t;

static unit_t *unit_new(const driver_t *driver, reports_t *reports, file_t *file, void *node) {
    unit_t *unit = NEW(unit_t);
    unit->driver = driver;
    unit->reports = reports;
    unit->file = file;
    unit->root = node;
    unit->mod = NULL;
    return unit;
}

int main(int argc, char **argv) {
    name = argv[0];
    init_memory();

    errors = begin_reports();

    settings_t settings = parse_args(errors, argc, argv);

    int err = end_reports(errors, SIZE_MAX, "command line parsing");
    if (err > 0) {
        return err;
    }

    size_t len = vector_len(settings.sources);
    vector_t *units = vector_new(len);

    int fails = 0;

    for (size_t i = 0; i < len; i++) {
        file_t *fp = vector_get(settings.sources, i);

        reports_t *reports = begin_reports();
        const driver_t *driver = select_driver_by_extension(reports, settings.driver, fp->path);

        if (driver == NULL) {
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

        module_t *mod = module_build(unit->reports, unit->lir);
        module_print(stdout, mod);

        err = end_reports(unit->reports, SIZE_MAX, format("code generation of `%s`", unit->file->path));
        fails = MAX(fails, err);

        unit->mod = mod;
    }

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);
        module_t *mod = unit->mod;
        const char *path = unit->file->path;

        gccjit_build(unit->reports, mod, format("%s.exe", path));
    
        err = end_reports(unit->reports, SIZE_MAX, format("code generation of `%s`", path));
        fails = MAX(fails, err);
    }

    return fails;
}
