#include "mediator.h"

#include "cthulhu/runtime/driver.h"

#include "cthulhu/tree/query.h"

#include "arena/arena.h"

#include "base/panic.h"

#include "scan/node.h"

#include "std/str.h"
#include "std/vector.h"

static char *path_to_string(vector_t *path, arena_t *arena)
{
    CTASSERT(path != NULL);
    CTASSERT(vector_len(path) > 0);

    return str_join(".", path, arena);
}

static context_t *context_inner_new(driver_t *handle, const char *name, void *ast, tree_t *root)
{
    CTASSERT(handle != NULL);
    CTASSERT(name != NULL);

    lifetime_t *lifetime = handle->parent;
    context_t *self = ARENA_MALLOC(sizeof(context_t), name, lifetime, lifetime->arena);

    self->parent = lifetime;
    self->lang = handle->lang;
    self->name = name;
    self->ast = ast;
    self->tree = root;

    return self;
}

USE_DECL
context_t *compiled_new(driver_t *handle, tree_t *root)
{
    return context_inner_new(handle, tree_get_name(root), NULL, root);
}

USE_DECL
context_t *context_new(driver_t *handle, const char *name, void *ast, tree_t *root)
{
    return context_inner_new(handle, name, ast, root);
}

USE_DECL
context_t *add_context(lifetime_t *lifetime, vector_t *path, context_t *mod)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(mod != NULL);

    char *name = path_to_string(path, lifetime->arena);

    context_t *old = map_get(lifetime->modules, name);
    if (old != NULL)
    {
        return old;
    }

    map_set(lifetime->modules, name, mod);
    return NULL;
}

USE_DECL
context_t *get_context(lifetime_t *lifetime, vector_t *path)
{
    CTASSERT(lifetime != NULL);

    char *name = path_to_string(path, lifetime->arena);

    return map_get(lifetime->modules, name);
}

USE_DECL
logger_t *lifetime_get_logger(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    return lifetime->logger;
}

USE_DECL
arena_t *lifetime_get_arena(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    return lifetime->arena;
}

USE_DECL
tree_cookie_t *lifetime_get_cookie(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    return &lifetime->cookie;
}

USE_DECL
void *context_get_ast(const context_t *context)
{
    CTASSERT(context != NULL);

    return context->ast;
}

USE_DECL
tree_t *context_get_module(const context_t *context)
{
    CTASSERT(context != NULL);

    return context->tree;
}

USE_DECL
lifetime_t *context_get_lifetime(const context_t *context)
{
    CTASSERT(context != NULL);

    return context->parent;
}

USE_DECL
const char *context_get_name(const context_t *context)
{
    CTASSERT(context != NULL);

    return context->name;
}

USE_DECL
void context_update(context_t *ctx, void *ast, tree_t *root)
{
    CTASSERT(ctx != NULL);

    ctx->ast = ast;
    ctx->tree = root;
}

///
/// helpers
///

USE_DECL
tree_t *lifetime_sema_new(lifetime_t *lifetime, const char *name, size_t len, const size_t *sizes)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(name != NULL);
    CTASSERT(sizes != NULL);
    CTASSERT(len > 0);

    logger_t *reports = lifetime_get_logger(lifetime);
    tree_cookie_t *cookie = lifetime_get_cookie(lifetime);
    arena_t *arena = lifetime_get_arena(lifetime);
    tree_t *root = tree_module_root(reports, cookie, node_builtin(), name, len, sizes, arena);

    return root;
}
