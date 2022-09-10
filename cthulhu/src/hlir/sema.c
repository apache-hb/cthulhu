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

    void *data;
} sema_t;

sema_t *sema_new(sema_t *parent, reports_t *reports, size_t decls, size_t *sizes)
{
    reports_t *innerReports = parent != NULL ? parent->reports : reports;

    CTASSERT(innerReports != NULL);

    sema_t *sema = ctu_malloc(sizeof(sema_t));

    sema->parent = parent;
    sema->reports = innerReports;

    sema->decls = vector_of(decls);
    for (size_t i = 0; i < decls; i++)
    {
        map_t *map = map_optimal(sizes[i]);
        vector_set(sema->decls, i, map);
    }

    return sema;
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
