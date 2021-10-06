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

int common_main(const frontend_t *frontend, int argc, char **argv) {
    init_memory();

    int error = 0;
    reports_t *errors = begin_reports();
    settings_t settings = parse_args(errors, frontend, argc, argv);

    if ((error = end_reports(errors, SIZE_MAX, "command line parsing")) > 0) {
        return error;
    }

    vector_t *sources = settings.sources;
    size_t len = vector_len(sources);

    vector_t *all = vector_of(len);

    for (size_t i = 0; i < len; i++) {
        file_t *file = vector_get(sources, i);
        reports_t *reports = begin_reports();
        void *node = frontend->parse(reports, file);
        context_t *ctx = new_context(reports, file, node);
        vector_set(all, i, ctx);

        error = MAX(error, end_reports(reports, SIZE_MAX, format("parsing of `%s`", file->path)));
    }

    if (error > 0) { return error; }

    for (size_t i = 0; i < len; i++) {
        context_t *ctx = vector_get(all, i);
        ctx->lir = frontend->analyze(ctx->reports, ctx->node);

        error = MAX(error, end_reports(ctx->reports, SIZE_MAX, format("analysis of `%s`", ctx->file->path)));
    }

    if (error > 0) { return error; }

    for (size_t i = 0; i < len; i++) {
        context_t *ctx = vector_get(all, i);

        ctx->mod = module_build(ctx->reports, ctx->lir);

        int local = end_reports(ctx->reports, SIZE_MAX, format("compilation of `%s`", ctx->file->path));
        if (local > 0) {
            error = MAX(error, local);
            continue;
        }

        eval_world(ctx->reports, ctx->mod);

        error = MAX(error, end_reports(ctx->reports, SIZE_MAX, format("evaluation of `%s`", ctx->file->path)));
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
        error = MAX(error, end_reports(ctx->reports, SIZE_MAX, format("generation of `%s`", ctx->file->path)));
    }

    return error;
}
