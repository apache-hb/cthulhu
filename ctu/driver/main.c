#include "ctu/util/report.h"
#include "ctu/util/str.h"

#include "driver.h"
#include "cmd.h"
#include "ctu/lir/sema.h"
#include "ctu/gen/emit.h"

#include "ctu/perf/perf.h"

#include <string.h>
#include <stdlib.h>

static reports_t *errors = NULL;

static const char *name = NULL;

typedef struct {
    const frontend_t *frontend;
    reports_t *reports;
    file_t *file;
    void *root;
    lir_t *lir;
    module_t *mod;
} unit_t;

static unit_t *unit_new(const frontend_t *frontend, 
                        reports_t *reports, 
                        file_t *file, 
                        void *node) {

    unit_t *unit = ctu_malloc(sizeof(unit_t));
    unit->frontend = frontend;
    unit->reports = reports;
    unit->file = file;
    unit->root = node;
    unit->mod = NULL;
    
    return unit;
}

static void unit_delete(unit_t *unit) {
    delete_reports(unit->reports);
    if (unit->mod != NULL) {
        module_free(unit->mod);
    }
    ctu_close(unit->file);
    ctu_free(unit, sizeof(unit_t));
}

int main(int argc, char **argv) {
    name = argv[0];
    init_memory();

    errors = begin_reports();

    settings_t settings = parse_args(errors, argc, argv);

    int err = end_reports(errors, SIZE_MAX, "command line parsing");

    delete_reports(errors);

    if (err > 0) {
        return err;
    }

    size_t len = vector_len(settings.sources);
    vector_t *units = vector_new(len);

    int fails = 0;

    for (size_t i = 0; i < len; i++) {
        file_t *fp = vector_get(settings.sources, i);

        reports_t *reports = begin_reports();
        const frontend_t *frontend = select_frontend_by_extension(reports, settings.frontend, fp->path);

        if (frontend == NULL) {
            report2(reports, ERROR, NULL, "unknown file type: %s", fp->path);
        } else {
            void *node = frontend->parse(reports, fp);
            vector_push(&units, unit_new(frontend, reports, fp, node));
        }

        err = end_reports(reports, SIZE_MAX, format("compilation of `%s`", fp->path));
        fails = MAX(fails, err);
    }

    if (fails > 0) {
        return fails;
    }

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);
        unit->lir = unit->frontend->analyze(unit->reports, unit->root);

        char *stage = format("semantic analysis of `%s`", unit->file->path);
        err = end_reports(unit->reports, SIZE_MAX, stage);
        fails = MAX(fails, err);
        ctu_free(stage, strlen(stage) + 1);
    }

    if (fails > 0) {
        return fails;
    }

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);

        module_t *mod = module_build(unit->reports, unit->lir);
        //module_print(stdout, mod);

        err = end_reports(unit->reports, SIZE_MAX, format("code generation of `%s`", unit->file->path));
        fails = MAX(fails, err);

        dead_function_elimination(unit->reports, mod);

        unit->mod = mod;
    }

    if (fails > 0) {
        return fails;
    }

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);
        module_t *mod = unit->mod;
        const char *path = unit->file->path;

        const backend_t *backend = settings.backend ?: &BACKEND_C99;

        if (backend->compile == NULL) {
            report2(unit->reports, NOTE, NULL, "backend `%s` is disabled", backend->name);
            continue;
        }

        backend->compile(unit->reports, mod, "out.c");
    
        char *stage = format("code generation of `%s`", path);
        err = end_reports(unit->reports, SIZE_MAX, stage);
        fails = MAX(fails, err);
        ctu_free(stage, strlen(stage) + 1);
    }

    if (fails > 0) {
        return fails;
    }

    vector_delete(settings.sources);

    for (size_t i = 0; i < len; i++) {
        unit_t *unit = vector_get(units, i);
        unit_delete(unit);
    }

    vector_delete(units);

    return fails;
}
