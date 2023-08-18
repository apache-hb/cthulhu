#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name)
{
    for (size_t i = 0; i < len; i++)
    {
        tree_t *decl = tree_module_get(sema, tags[i], name);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}
