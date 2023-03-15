#include "cthulhu/hlir/sema.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"
#include "base/util.h"

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#include "report/report.h"

#include "std/map.h"

#include <string.h>

typedef struct sema_t
{
    struct sema_t *parent;
    reports_t *reports;

    /**
     * an array of maps
     * each map is its own namespace which maps from symbol name to hlir node
     */
    vector_t *decls;

    vector_t *stack; // recursion stack

    void *data;
} sema_t;

static sema_t *sema_inner_new(sema_t *parent, reports_t *reports, vector_t *stack, size_t decls, size_t *sizes)
{
    sema_t *sema = ctu_malloc(sizeof(sema_t));

    sema->parent = parent;
    sema->reports = reports;

    sema->stack = stack;
    sema->data = NULL;

    sema->decls = vector_of(decls);
    for (size_t i = 0; i < decls; i++)
    {
        map_t *map = map_optimal(sizes[i]);
        vector_set(sema->decls, i, map);
    }

    return sema;
}

sema_t *sema_root_new(reports_t *reports, size_t decls, size_t *sizes)
{
    CTASSERT(reports != NULL);
    return sema_inner_new(NULL, reports, vector_new(16), decls, sizes);
}

sema_t *sema_new(sema_t *parent, size_t decls, size_t *sizes)
{
    CTASSERT(parent != NULL);
    return sema_inner_new(parent, parent->reports, parent->stack, decls, sizes);
}

reports_t *sema_reports(sema_t *sema)
{
    CTASSERT(sema != NULL);
    return sema->reports;
}

sema_t *sema_parent(sema_t *sema)
{
    CTASSERT(sema != NULL);
    return sema->parent;
}

void sema_delete(sema_t *sema)
{
    ctu_free(sema);
}

void sema_set_data(sema_t *sema, void *data)
{
    sema->data = data;
}

void *sema_get_data(sema_t *sema)
{
    return sema->data;
}

void sema_set(sema_t *sema, size_t tag, const char *name, void *data)
{
    map_t *map = sema_tag(sema, tag);
    map_set(map, name, data);
}

void *sema_get(sema_t *sema, size_t tag, const char *name)
{
    map_t *map = sema_tag(sema, tag);

    hlir_t *hlir = map_get(map, name);
    if (hlir != NULL)
    {
        return hlir;
    }

    if (sema->parent != NULL)
    {
        return sema_get(sema->parent, tag, name);
    }

    return NULL;
}

map_t *sema_tag(sema_t *sema, size_t tag)
{
    return vector_get(sema->decls, tag);
}

hlir_t *sema_resolve(sema_t *root, hlir_t *unresolved, sema_resolve_t resolve)
{
    CTASSERT(root->parent == NULL); // we want the root node

    // bail early if its already resolved
    if (!hlir_is(unresolved, eHlirUnresolved))
    {
        return unresolved;
    }

    if (vector_find(root->stack, unresolved) != SIZE_MAX)
    {
        // declaration requires recursive resolution, error out
        report(root->reports, eFatal, get_hlir_node(unresolved), "recursive resolution for %s", get_hlir_name(unresolved));
        return unresolved;
    }

    // save attributes
    const hlir_attributes_t *attribs = get_hlir_attributes(unresolved);

    vector_push(&root->stack, unresolved);

    hlir_t *result = resolve(unresolved->sema, unresolved->user);
    memcpy(unresolved, result, sizeof(hlir_t));
    
    vector_drop(root->stack);

    hlir_set_attributes(unresolved, attribs);

    return unresolved;
}
