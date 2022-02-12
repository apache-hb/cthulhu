#include "driver.h"
#include "cmd.h"

#include "cthulhu/util/str.h"

#include "cthulhu/gen/emit.h"
#include "cthulhu/gen/eval.h"

#include "cthulhu/perf/perf.h"

#include <string.h>

#include "cthulhu/backend/c99/c99.h"
#include "cthulhu/backend/json/json.h"

const backend_t BACKEND_C99 = {
    .version = "0.0.1",
    .name = "C99",
    .compile = (compile_t)c99_build
};

const backend_t BACKEND_JSON = {
    .version = "0.0.1",
    .name = "JSON",
    .compile = (compile_t)json_build
};

arg_t *new_arg(
    arg_type_t type, 
    const char *name, 
    const char *brief, 
    const char *desc
)
{
    arg_t *arg = ctu_malloc(sizeof(arg_t));

    arg->type = type;
    arg->name = name;
    arg->brief = brief;
    arg->desc = desc;

    return arg;
}

const backend_t *select_backend(reports_t *reports, const char *name) {
    if (name == NULL) {
        report(reports, ERROR, NULL, "no backend specified");
    }

    if (streq(name, "c99")) {
        return &BACKEND_C99;
    } else if (streq(name, "json")) {
        return &BACKEND_JSON;
    } else {
        report(reports, ERROR, NULL, "unknown backend: %s", name);
        return NULL;
    }
}

typedef struct {
    reports_t *reports;
    scan_t scan;
    file_t *file;
    void *node;
    vector_t *modules;
    module_t *mod;
} context_t;

static int max_report(int *error, reports_t *reports, const char *msg) {
    int result = end_reports(reports, SIZE_MAX, msg);
    *error = MAX(*error, result);
    return result;
}

int common_main(const frontend_t *frontend, int argc, char **argv) {
    init_gmp();

    int error = 0;
    reports_t *errors = begin_reports();
    settings_t settings = parse_args(errors, frontend, argc, argv);
    verbose = settings.verbose;

    if ((error = end_reports(errors, SIZE_MAX, "command line parsing")) > 0) {
        return error;
    }

    if (frontend->init != NULL) {
        frontend->init();
    }

#if FUZZING
    while (__AFL_LOOP(1000)) {
#endif

    vector_t *sources = settings.sources;
    size_t len = vector_len(sources);
    context_t *all = ALLOCA(sizeof(context_t) * len);

    logverbose("compiling %zu file(s)", len);

    for (size_t i = 0; i < len; i++) {
        context_t *ctx = all + i;
        ctx->reports = begin_reports();
        ctx->file = vector_get(sources, i);
        ctx->scan = frontend->open(ctx->reports, ctx->file);
        ctx->node = frontend->parse(&ctx->scan);

        max_report(&error, ctx->reports, format("parsing of `%s`", ctx->file->path));
    }

    logverbose("parsed %zu file(s)", len);

    if (error > 0) { return error; }

    for (size_t i = 0; i < len; i++) {
        context_t *ctx = all + i;
        ctx->modules = frontend->analyze(ctx->reports, &settings, ctx->node);

        max_report(&error, ctx->reports, format("analysis of `%s`", ctx->file->path));
    }

    logverbose("analyzed %zu file(s)", len);

    if (error > 0) { return error; }

    for (size_t i = 0; i < len; i++) {
        context_t *ctx = all + i;

        ctx->mod = module_build(ctx->reports, ctx->file->path, ctx->modules);

        logverbose("reporting %s", ctx->file->path);
        int local = max_report(&error, ctx->reports, format("compilation of `%s`", ctx->file->path));
        if (local > 0) {
            continue;
        }

        eval_world(ctx->reports, ctx->mod);
        run_passes(ctx->reports, ctx->mod);
        validate_world(ctx->reports, ctx->mod);

        if (settings.ir) {
            module_print(stdout, ctx->mod);
        }

        max_report(&error, ctx->reports, format("evaluation of `%s`", ctx->file->path));
    }

    if (error > 0) { return error; }

    const backend_t *backend = settings.backend;
    for (size_t i = 0; i < len; i++) {
        context_t *ctx = all + i;
        if (backend == NULL || backend->compile == NULL) {
            report(ctx->reports, NOTE, NULL, "no backend specified, skipping compilation of `%s`", ctx->file->path);
            continue;
        }

        backend->compile(ctx->reports, ctx->mod, settings.output != NULL ? format("%s.c", settings.output) : "out.c");
        max_report(&error, ctx->reports, format("generation of `%s`", ctx->file->path));
    }

#if FUZZING
    }
#endif

    return error;
}