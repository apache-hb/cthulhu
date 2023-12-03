#include "common.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/tree/query.h"

#include "memory/memory.h"

#include "base/panic.h"

#include "std/str.h"
#include "std/vector.h"

#include "report/report.h"

static char *path_to_string(vector_t *path)
{
    CTASSERT(path != NULL);
    CTASSERT(vector_len(path) > 0);

    return str_join(".", path);
}

static context_t *context_inner_new(driver_t *handle, const char *name, void *ast, tree_t *root)
{
    CTASSERT(handle != NULL);
    context_t *self = ctu_malloc(sizeof(context_t));

    self->parent = handle->parent;
    self->lang = handle->lang;
    self->name = name;
    self->ast = ast;
    self->root = root;

    return self;
}

context_t *compiled_new(driver_t *handle, tree_t *root)
{
    return context_inner_new(handle, tree_get_name(root), NULL, root);
}

context_t *context_new(driver_t *handle, const char *name, void *ast, tree_t *root)
{
    return context_inner_new(handle, name, ast, root);
}

context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(mod != NULL);

    char *name = path_to_string(path);

    logverbose("add-context (%s: 0x%p)", name, (void*)mod);

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

USE_DECL
reports_t *lifetime_get_reports(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    return lifetime->reports;
}

USE_DECL
cookie_t *lifetime_get_cookie(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    return lifetime->cookie;
}

void *context_get_ast(context_t *context)
{
    CTASSERT(context != NULL);

    return context->ast;
}

tree_t *context_get_module(context_t *context)
{
    CTASSERT(context != NULL);

    return context->root;
}

lifetime_t *context_get_lifetime(context_t *context)
{
    CTASSERT(context != NULL);

    return context->parent;
}

const char *context_get_name(context_t *context)
{
    CTASSERT(context != NULL);

    return context->name;
}

void context_update(context_t *ctx, void *ast, tree_t *root)
{
    CTASSERT(ctx != NULL);

    ctx->ast = ast;
    ctx->root = root;
}

///
/// helpers
///

tree_t *lifetime_sema_new(lifetime_t *lifetime, const char *name, size_t len, const size_t *sizes)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(name != NULL);
    CTASSERT(sizes != NULL);
    CTASSERT(len > 0);

    reports_t *reports = lifetime_get_reports(lifetime);
    cookie_t *cookie = lifetime_get_cookie(lifetime);
    tree_t *root = tree_module_root(reports, cookie, node_builtin(), name, len, sizes);

    return root;
}
