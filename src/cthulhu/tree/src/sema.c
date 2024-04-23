// SPDX-License-Identifier: LGPL-3.0-only

#include "arena/arena.h"
#include "common.h"

#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/map.h"

#include "memory/memory.h"
#include "base/panic.h"

static tree_t *tree_module_new(const node_t *node, const char *name,
                               tree_t *parent, tree_cookie_t *cookie,
                               logger_t *reports,
                               size_t decls, const size_t *sizes,
                               arena_t *arena)
{
    CTASSERTF(decls >= eSemaCount, "module cannot be constructed with less than %d tags (%zu given)", eSemaCount, decls);
    CTASSERT(reports != NULL);

    tree_t *self = tree_decl(eTreeDeclModule, node, NULL, name, eQualNone);
    self->arena = arena;
    self->parent = parent;
    self->cookie = cookie;
    self->reports = reports;

    self->tags = vector_of(decls, arena);
    ARENA_IDENTIFY(self->tags, "module_tags", self, arena);

    for (size_t i = 0; i < decls; i++)
    {
        map_t *map = map_optimal(sizes[i], kTypeInfoString, arena);
        ARENA_IDENTIFY(map, "module_tag", self, arena);
        vector_set(self->tags, i, map);
    }

    return self;
}

tree_t *tree_module_root(logger_t *reports, tree_cookie_t *cookie, const node_t *node, const char *name, size_t decls, const size_t *sizes, arena_t *arena)
{
    return tree_module_new(
        node, name,
        /* parent = */ NULL,
        /* cookie = */ cookie,
        /* reports = */ reports,
        decls, sizes, arena);
}

tree_t *tree_module(tree_t *parent, const node_t *node, const char *name, size_t decls, const size_t *sizes)
{
    TREE_EXPECT(parent, eTreeDeclModule);

    return tree_module_new(node, name, parent, parent->cookie, parent->reports, decls, sizes, parent->arena);
}

void *tree_module_get(tree_t *self, size_t tag, const char *name)
{
    CTASSERT(name != NULL);

    // its ok to do an early return here and skip checking the parent module
    // because parent modules will always have <= the tags of the child module
    map_t *map = tree_module_tag(self, tag);
    if (map == NULL) return NULL;

    tree_t *old = map_get(map, name);
    if (old != NULL)
    {
        return old;
    }

    if (self->parent != NULL)
    {
        return tree_module_get(self->parent, tag, name);
    }

    return NULL;
}

void *tree_module_find(tree_t *sema, size_t tag, const char *name, tree_t **module)
{
    CTASSERT(sema != NULL);
    CTASSERT(name != NULL);
    CTASSERT(module != NULL);

    map_t *map = tree_module_tag(sema, tag);
    if (map == NULL)
    {
        *module = NULL;
        return NULL;
    }

    tree_t *decl = map_get(map, name);
    if (decl != NULL)
    {
        *module = sema;
        return decl;
    }

    if (sema->parent != NULL)
    {
        return tree_module_find(sema->parent, tag, name, module);
    }

    *module = NULL;
    return NULL;
}

void *tree_module_set(tree_t *self, size_t tag, const char *name, void *value)
{
    void *old = tree_module_get(self, tag, name);
    if (old != NULL)
    {
        return old;
    }

    map_t *map = tree_module_tag(self, tag);
    map_set(map, name, value);

    return NULL;
}

map_t *tree_module_tag(const tree_t *self, size_t tag)
{
    TREE_EXPECT(self, eTreeDeclModule);

    size_t len = vector_len(self->tags);
    if (tag >= len)
        return NULL;

    return vector_get(self->tags, tag);
}

tree_cookie_t *tree_get_cookie(tree_t *self)
{
    TREE_EXPECT(self, eTreeDeclModule);

    return self->cookie;
}
