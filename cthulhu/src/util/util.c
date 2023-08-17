#include "cthulhu/util/util.h"

#include "cthulhu/hlir/query.h"

void *util_select_decl(h2_t *sema, const size_t *tags, size_t len, const char *name)
{
    for (size_t i = 0; i < len; i++)
    {
        h2_t *decl = h2_module_get(sema, tags[i], name);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}
