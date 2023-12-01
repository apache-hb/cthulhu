#include "cthulhu/util/util.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "std/vector.h"

#include "base/panic.h"

static tree_t *select_module(tree_t *sema, const util_search_t *search, const char *name, bool *imported)
{
    CTASSERT(imported != NULL);

    tree_t *inner = util_select_decl(sema, search->localScopeTags, search->localScopeTagsLen, name);
    if (inner != NULL) { return inner; }

    tree_t *global = util_select_decl(sema, search->globalScopeTags, search->globalScopeTagsLen, name);
    if (global != NULL)
    {
        *imported = true;
        return global;
    }

    return NULL;
}

static bool is_public(const tree_t *decl)
{
    const tree_attribs_t *attrib = tree_get_attrib(decl);
    return attrib->visibility == eVisiblePublic;
}

tree_t *util_search_namespace(tree_t *sema, const util_search_t *search, const node_t *node, vector_t *path, bool *isImported)
{
    CTASSERTF(sema != NULL && search != NULL, "(sema = %p, search = %p)", (void*)sema, (void*)search);
    CTASSERT(vector_len(path) > 0);
    CTASSERT(isImported != NULL);

    size_t len = vector_len(path);
    tree_t *ns = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        if (!tree_is(ns, eTreeDeclModule))
        {
            return tree_raise(node, sema->reports, "expected a namespace but got `%s` instead", tree_to_string(ns));
        }

        const char *segment = vector_get(path, i);
        ns = select_module(ns, search, segment, isImported);
        if (ns == NULL)
        {
            return tree_raise(node, sema->reports, "namespace `%s` not found", segment);
        }
    }

    return ns;
}

tree_t *util_search_path(tree_t *sema, const util_search_t *search, const node_t *node, vector_t *path)
{
    CTASSERTF(sema != NULL && search != NULL, "(sema = %p, search = %p)", (void*)sema, (void*)search);
    CTASSERT(vector_len(path) > 0);

    bool isImported = false;
    size_t len = vector_len(path);
    tree_t *ns = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *segment = vector_get(path, i);
        ns = select_module(ns, search, segment, &isImported);
        if (ns == NULL)
        {
            return tree_raise(node, sema->reports, "namespace `%s` not found", segment);
        }
    }

    const char *name = vector_tail(path);
    tree_t *decl = util_select_decl(ns, search->declTags, search->declTagsLen, name);
    if (decl == NULL)
    {
        return tree_raise(node, sema->reports, "decl `%s` not found", name);
    }

    if (isImported && !is_public(decl))
    {
        return tree_raise(node, sema->reports, "decl `%s` is not public", name);
    }

    return decl;
}

tree_t *util_search_qualified(tree_t *sema, const util_search_t *search, const node_t *node, const char *mod, const char *name)
{
    bool isImported = false;
    tree_t *ns = select_module(sema, search, mod, &isImported);
    if (ns == NULL)
    {
        return tree_raise(node, sema->reports, "namespace `%s` not found", mod);
    }

    tree_t *decl = util_select_decl(ns, search->declTags, search->declTagsLen, name);
    if (decl == NULL)
    {
        return tree_raise(node, sema->reports, "decl `%s` not found", name);
    }

    if (isImported && !is_public(decl))
    {
        return tree_raise(node, sema->reports, "decl `%s` is not public", name);
    }

    return decl;
}
