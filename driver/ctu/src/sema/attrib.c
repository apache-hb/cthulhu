#include "ctu/sema/attrib.h"
#include "ctu/sema/decl.h"

#include "ctu/ast.h"

#include "cthulhu/tree/query.h"

#include "report/report.h"

#include "std/str.h"
#include "std/vector.h"

#include "base/memory.h"
#include "base/util.h"
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
    CTASSERT(name != NULL);
    CTASSERT(fnApply != NULL);

    ctu_attrib_t *it = ctu_malloc(sizeof(ctu_attrib_t));
    it->name = name;
    it->fnApply = fnApply;
    return it;
}

#define MALFORMED_ENTRY(REPORTS, NODE) report(REPORTS, eFatal, NODE, "malformed entry point type, must be either `gui` or `cli`")

static tree_link_t choose_linkage(tree_t *sema, tree_t *decl, const ctu_t *expr)
{
    if (expr->kind != eCtuExprName)
    {
        MALFORMED_ENTRY(sema->reports, expr->node);
        return eLinkEntryCli;
    }

    vector_t *path = expr->path;
    if (vector_len(path) > 1)
    {
        MALFORMED_ENTRY(sema->reports, expr->node);
        return eLinkEntryCli;
    }

    const char *name = vector_tail(path);
    if (str_equal(name, "gui"))
    {
        return eLinkEntryGui;
    }
    else if (str_equal(name, "cli"))
    {
        return eLinkEntryCli;
    }

    MALFORMED_ENTRY(sema->reports, expr->node);
    return eLinkEntryCli;
}

static tree_link_t get_linkage(tree_t *sema, tree_t *decl, vector_t *args)
{
    switch (vector_len(args))
    {
    case 0: return eLinkEntryCli;
    default:
        report(sema->reports, eWarn, tree_get_node(decl), "entry attribute takes at most 1 argument, ignoring extra arguments");
    case 1:
        return choose_linkage(sema, decl, vector_tail(args));
    }
}

static void apply_entry(tree_t *sema, tree_t *decl, vector_t *args)
{
    if (!tree_is(decl, eTreeDeclFunction))
    {
        report(sema->reports, eFatal, decl->node, "entry attribute can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->link != eLinkModule && old->link != eLinkExport)
    {
        report(sema->reports, eFatal, decl->node, "entry attribute can only be applied to public functions");
        return;
    }

    tree_attribs_t *copy = ctu_memdup(old, sizeof(tree_attribs_t));
    copy->link = get_linkage(sema, decl, args);
    tree_set_attrib(decl, copy);
}

static void apply_deprecated(tree_t *sema, tree_t *decl, vector_t *args)
{
    NEVER("not implemented");
}

static void apply_section(tree_t *sema, tree_t *decl, vector_t *args)
{
    NEVER("not implemented");
}

void ctu_init_attribs(tree_t *sema)
{
    ctu_attrib_t *entry = attrib_create("entry", apply_entry);
    tree_module_set(sema, eCtuTagAttribs, entry->name, entry);

    ctu_attrib_t *deprecated = attrib_create("deprecated", apply_deprecated);
    tree_module_set(sema, eCtuTagAttribs, deprecated->name, deprecated);

    ctu_attrib_t *section = attrib_create("section", apply_section);
    tree_module_set(sema, eCtuTagAttribs, section->name, section);
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

        it->fnApply(sema, decl, attrib->attribArgs);
    }
}
