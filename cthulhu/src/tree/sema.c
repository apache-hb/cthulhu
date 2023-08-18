#include "common.h"

#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/map.h"

#include "base/memory.h"
#include "base/panic.h"

#include "report/report.h"

static tree_t *tree_module_new(const node_t *node, const char *name, tree_t *parent, cookie_t *cookie, reports_t *reports, size_t decls, const size_t *sizes)
{
    CTASSERTF(decls >= eSema2Total, "module cannot be constructed with less than %zu tags (%zu given)", eSema2Total, decls);
    CTASSERT(reports != NULL);

    tree_t *self = tree_decl(eTreeDeclModule, node, NULL, name);
    self->parent = parent;
    self->cookie = cookie;
    self->reports = reports;
    self->tags = vector_of(decls);

    for (size_t i = 0; i < decls; i++)
    {
        map_t *map = map_optimal(sizes[i]);
        vector_set(self->tags, i, map);
    }

    return self;
}

tree_t *tree_module_root(reports_t *reports, cookie_t *cookie, const node_t *node, const char *name, size_t decls, const size_t *sizes)
{
    return tree_module_new(node, name, NULL, cookie, reports, decls, sizes);
}

tree_t *tree_module(tree_t *parent, const node_t *node, const char *name, size_t decls, const size_t *sizes)
{
    CTASSERT(parent != NULL);

    return tree_module_new(node, name, parent, parent->cookie, parent->reports, decls, sizes);
}

void *tree_module_get(tree_t *self, size_t tag, const char *name)
{
    CTASSERT(name != NULL);

    map_t *map = tree_module_tag(self, tag);
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
    CTASSERT(self != NULL);

    return vector_get(self->tags, tag);
}

cookie_t *tree_get_cookie(tree_t *sema)
{
    CTASSERT(tree_is(sema, eTreeDeclModule));

    return sema->cookie;
}
