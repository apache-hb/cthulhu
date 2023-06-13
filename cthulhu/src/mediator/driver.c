#include "common.h"

#include "cthulhu/mediator/driver.h"

#include "base/panic.h"
#include "base/memory.h"

#include "std/str.h"
#include "std/vector.h"

#include "report/report.h"

static char *path_to_string(vector_t *path)
{
    CTASSERT(path != NULL);
    CTASSERT(vector_len(path) > 0);

    return str_join(".", path);
}

context_t *context_new(handle_t *handle, const char *name, void *ast, hlir_t *root, sema_t *sema)
{
    CTASSERT(handle != NULL);

    context_t *self = ctu_malloc(sizeof(context_t));

    self->parent = handle->parent;
    self->lang = handle->lang;
    self->name = name;
    self->ast = ast;
    self->root = root;
    self->sema = sema;

    return self;
}

context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(mod != NULL);

    char *name = path_to_string(path);

    logverbose("add-context (%s: 0x%p)", name, mod);

    context_t *old = map_get(lifetime->modules, name);
    if (old != NULL)
    {
        return old;
    }

    map_set(lifetime->modules, name, mod);
    return NULL;
}

context_t *get_context(lifetime_t *lifetime, vector_t *path)
{
    CTASSERT(lifetime != NULL);

    char *name = path_to_string(path);

    return map_get(lifetime->modules, name);
}

reports_t *lifetime_get_reports(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    return lifetime->reports;
}

void *context_get_ast(context_t *context)
{
    CTASSERT(context != NULL);

    return context->ast;
}

hlir_t *context_get_hlir(context_t *context)
{
    CTASSERT(context != NULL);

    return context->root;
}

sema_t *context_get_sema(context_t *context)
{
    CTASSERT(context != NULL);

    return context->sema;
}

lifetime_t *context_get_lifetime(context_t *context)
{
    CTASSERT(context != NULL);

    return context->parent;
}

const char *context_get_name(context_t *context)
{
    CTASSERT(context != NULL);

    return "";
}

void context_update(context_t *ctx, void *ast, sema_t *sema, hlir_t *root)
{
    CTASSERT(ctx != NULL);

    ctx->ast = ast;
    ctx->root = root;
    ctx->sema = sema;
}
