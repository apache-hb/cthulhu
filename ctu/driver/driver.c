#include "driver.h"
#include "cmd.h"

#include "ctu/util/str.h"

#include "ctu/gen/emit.h"
#include "ctu/gen/eval.h"

#include <string.h>

#include "ctu/backend/c99/c99.h"
#include "ctu/backend/gcc/gcc.h"
#include "ctu/backend/llvm/llvm.h"
#include "ctu/backend/null/null.h"

const backend_t BACKEND_LLVM = {
    .version = "0.0.1",
    .name = "LLVM",
    .compile = NULL //(compile_t)llvm_build
};

const backend_t BACKEND_C99 = {
    .version = "0.0.1",
    .name = "C99",
    .compile = (compile_t)c99_build
};

const backend_t BACKEND_GCCJIT = {
    .version = "0.0.1",
    .name = "GCCJIT",
    .compile = NULL //(compile_t)gccjit_build
};

const backend_t BACKEND_NULL = {
    .version = "1.0.0",
    .name = "NULL",
    .compile = (compile_t)null_build
};
const backend_t *select_backend(reports_t *reports, const char *name) {
    if (name == NULL) {
        report(reports, ERROR, NULL, "no backend specified");
    }

    if (streq(name, "c99")) {
        return &BACKEND_C99;
    } else if (streq(name, "gccjit")) {
        return &BACKEND_GCCJIT;
    } else if (streq(name, "llvm")) {
        return &BACKEND_LLVM;
    } else if (streq(name, "null")) {
        return &BACKEND_NULL;
    } else {
        report(reports, ERROR, NULL, "unknown backend: %s", name);
        return NULL;
    }
}

typedef struct {
    reports_t *reports;
    file_t *file;
    void *node;
    lir_t *lir;
    module_t *mod;
} context_t;

static context_t *new_context(reports_t *reports, file_t *file, void *node) {
    context_t *ctx = ctu_malloc(sizeof(context_t));
    ctx->reports = reports;
    ctx->file = file;
    ctx->node = node;
    return ctx;
}

static int max_report(int *error, reports_t *reports, const char *msg) {
    int result = end_reports(reports, SIZE_MAX, msg);
    *error = MAX(*error, result);
    return result;
}

int common_main(const frontend_t *frontend, int argc, char **argv, void(*init)(void)) {
    init_memory();

    int error = 0;
    arena_t report_arena = new_blockmap(sizeof(message_t), 128);
    reports_t *errors = begin_reports(&report_arena);
    settings_t settings = parse_args(errors, frontend, argc, argv);
    verbose = settings.verbose;

    if ((error = end_reports(errors, SIZE_MAX, "command line parsing")) > 0) {
        return error;
    }

    if (init != NULL) {
        init();
    }

#if FUZZING
    while (__AFL_LOOP(1000)) {
#endif

    vector_t *sources = settings.sources;
    size_t len = vector_len(sources);

    logverbose("compiling %zu file(s)", len);

    vector_t *all = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        file_t *file = vector_get(sources, i);
        reports_t *reports = begin_reports(&report_arena);
        void *node = frontend->parse(reports, file);
        context_t *ctx = new_context(reports, file, node);
        vector_set(all, i, ctx);

        max_report(&error, reports, format("parsing of `%s`", file->path));
    }

    if (error > 0) { return error; }

    for (size_t i = 0; i < len; i++) {
        context_t *ctx = vector_get(all, i);
        ctx->lir = frontend->analyze(ctx->reports, ctx->node);

        max_report(&error, ctx->reports, format("analysis of `%s`", ctx->file->path));
    }

    if (error > 0) { return error; }

    for (size_t i = 0; i < len; i++) {
        context_t *ctx = vector_get(all, i);

        ctx->mod = module_build(ctx->reports, ctx->lir);

        logverbose("reporting %s", ctx->file->path);
        int local = max_report(&error, ctx->reports, format("compilation of `%s`", ctx->file->path));
        if (local > 0) {
            continue;
        }

        eval_world(ctx->reports, ctx->mod);

        if (settings.ir) {
            module_print(stdout, ctx->mod);
        }

        max_report(&error, ctx->reports, format("evaluation of `%s`", ctx->file->path));
    }

    if (error > 0) { return error; }

    const backend_t *backend = settings.backend;
    for (size_t i = 0; i < len; i++) {
        context_t *ctx = vector_get(all, i);
        if (backend == NULL || backend->compile == NULL) {
            report(ctx->reports, NOTE, NULL, "no backend specified, skipping compilation of `%s`", ctx->file->path);
            continue;
        }

        backend->compile(ctx->reports, ctx->mod, "out.c");
        max_report(&error, ctx->reports, format("generation of `%s`", ctx->file->path));
    }

#if FUZZING
    }
#endif

    return error;
}
