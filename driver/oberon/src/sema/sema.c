#include "oberon/sema/decl.h"

#include "report/report-ext.h"

static h2_t *obr_get_decl(h2_t *sema, const size_t *tags, size_t len, const char *name)
{
    for (size_t i = 0; i < len; i++)
    {
        size_t tag = tags[i];
        h2_t *decl = h2_module_get(sema, tag, name);
        if (decl != NULL) { return decl; }
    }
    return NULL;
}

h2_t *obr_get_type(h2_t *sema, const char *name)
{
    const size_t tags[] = { eTagTypes };
    return obr_get_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

h2_t *obr_get_module(h2_t *sema, const char *name)
{
    const size_t tags[] = { eTagModules };
    return obr_get_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

void obr_add_decl(h2_t *sema, obr_tags_t tag, const char *name, h2_t *decl)
{
    const h2_t *old = h2_module_get(sema, tag, name);
    if (old != NULL)
    {
        report_shadow(sema->reports, name, h2_get_node(old), h2_get_node(decl));
    }
    else
    {
        h2_module_set(sema, tag, name, decl);
    }
}
