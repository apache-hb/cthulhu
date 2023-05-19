#include "cthulhu/mediator/language.h"

#include "common.h"

#include "base/memory.h"
#include "base/panic.h"

#include "report/report.h"

typedef struct lang_handle_t
{
    lifetime_t *parent;
    const language_t *language;

    void *user;
} lang_handle_t;

typedef struct compile_t
{
    sema_t *sema;
    void *ast;
    hlir_t *mod;
} compile_t;

lang_handle_t *lang_init(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    lang_handle_t *handle = ctu_malloc(sizeof(lang_handle_t));
    handle->parent = lifetime;
    handle->language = lang;
    handle->user = NULL;

    EXEC(lang, fnInit, handle);

    return handle;
}

compile_t *compile_init(lang_handle_t *handle, void *ast, sema_t *sema, hlir_t *mod)
{
    CTASSERT(handle != NULL);
    CTASSERT(sema != NULL);
    CTASSERT(mod != NULL);

    compile_t *compile = ctu_malloc(sizeof(compile_t));
    compile->sema = sema;
    compile->mod = mod;

    return compile;
}

void lang_compile(lang_handle_t *handle, compile_t *compile)
{
    CTASSERT(handle != NULL);
    CTASSERT(compile != NULL);

    EXEC(handle->language, fnCompile, handle, compile);
}

// driver api

sema_t *handle_get_sema(lang_handle_t *self, const char *mod)
{
    compile_t *compile = lifetime_get_module(self->parent, mod);
    if (compile == NULL) { return NULL; }

    return compile->sema;
}

reports_t *lang_get_reports(lang_handle_t *self)
{
    CTASSERT(self != NULL);

    return begin_reports(); // TODO
}

void *compile_get_ast(compile_t *self)
{
    CTASSERT(self != NULL);

    return self->ast;
}

sema_t *compile_get_sema(compile_t *self)
{
    CTASSERT(self != NULL);

    return self->sema;
}

hlir_t *compile_get_module(compile_t *self)
{
    CTASSERT(self != NULL);

    return self->mod;
}

void compile_begin(lang_handle_t *self, void *ast, const char *name, sema_t *sema, hlir_t *mod)
{
    compile_t *compile = compile_init(self, ast, sema, mod);

    lifetime_add_module(self->parent, self, name, compile);
}

void compile_finish(compile_t *self)
{
    // TODO: what goes here?
}
