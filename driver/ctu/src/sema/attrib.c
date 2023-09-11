#include "ctu/sema/attrib.h"
#include "ctu/sema/decl.h"

#include "ctu/ast.h"

#include "report/report.h"

#include "std/str.h"
#include "std/vector.h"

#include "base/memory.h"
#include "base/panic.h"

static const char *attrib_name(vector_t *path)
{
    return str_join("::", path);
}

static ctu_attrib_t *get_attrib(tree_t *sema, vector_t *path)
{
    CTASSERTF(vector_len(path) == 1, "expected 1 path element but got `%s`", attrib_name(path));
    const char *name = vector_tail(path);

    return ctu_get_attrib(sema, name);
}

static ctu_attrib_t *attrib_create(const char *name, ctu_attrib_apply_t fnApply)
{
    ctu_attrib_t *it = ctu_malloc(sizeof(ctu_attrib_t));
    it->name = name;
    it->fnApply = fnApply;
    return it;
}

static void apply_entry(tree_t *sema, tree_t *decl, vector_t *args)
{

}

static ctu_attrib_t *ctu_attrib_entry(void)
{
    return NULL;
}

void ctu_apply_attribs(tree_t *sema, tree_t *decl, vector_t *attribs)
{
    size_t len = vector_len(attribs);
    for (size_t i = 0; i < len; i++)
    {
        ctu_t *attrib = vector_get(attribs, i);
        CTASSERTF(attrib->kind == eCtuAttrib, "expected attrib but got %d", attrib->kind);

        ctu_attrib_t *it = get_attrib(sema, attrib->attribPath);
        if (it == NULL)
        {
            report(sema->reports, eWarn, attrib->node, "attrib '%s' not found", attrib_name(attrib->attribPath));
            continue;
        }
    }
}
