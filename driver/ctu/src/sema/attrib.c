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

static const char *get_first_string(tree_t *sema, tree_t *decl, vector_t *args)
{
    if (vector_len(args) != 1)
    {
        report(sema->reports, eFatal, decl->node, "expected 1 string argument");
        return NULL;
    }

    ctu_t *arg = vector_tail(args);
    if (arg->kind != eCtuExprString)
    {
        report(sema->reports, eFatal, arg->node, "expected string argument");
        return NULL;
    }

    return arg->text;
}

static const char *get_first_ident(tree_t *sema, tree_t *decl, vector_t *args)
{
    if (vector_len(args) != 1)
    {
        report(sema->reports, eFatal, decl->node, "expected 1 identifier argument");
        return NULL;
    }

    ctu_t *arg = vector_tail(args);
    if (arg->kind != eCtuExprName)
    {
        report(sema->reports, eFatal, arg->node, "expected identifier argument");
        return NULL;
    }

    if (vector_len(arg->path) != 1)
    {
        report(sema->reports, eFatal, arg->node, "expected identifier argument");
        return NULL;
    }

    return vector_tail(arg->path);
}

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

///
/// attributes
///

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
    if (!tree_is(decl, eTreeDeclFunction))
    {
        report(sema->reports, eFatal, decl->node, "deprecated attribute can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->deprecated != NULL)
    {
        report(sema->reports, eFatal, decl->node, "deprecated attribute already applied");
        return;
    }

    const char *msg = get_first_string(sema, decl, args);
    if (msg == NULL) { return; }

    tree_attribs_t *copy = ctu_memdup(old, sizeof(tree_attribs_t));
    copy->deprecated = msg;
    tree_set_attrib(decl, copy);
}

static void apply_section(tree_t *sema, tree_t *decl, vector_t *args)
{
    if (!tree_is(decl, eTreeDeclFunction))
    {
        report(sema->reports, eFatal, decl->node, "section attribute can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->section != NULL)
    {
        report(sema->reports, eFatal, decl->node, "section attribute already applied");
        return;
    }

    const char *msg = get_first_string(sema, decl, args);
    if (msg == NULL) { return; }

    tree_attribs_t *copy = ctu_memdup(old, sizeof(tree_attribs_t));
    copy->section = msg;
    tree_set_attrib(decl, copy);
}

static void apply_extern(tree_t *sema, tree_t *decl, vector_t *args)
{
    if (!tree_is(decl, eTreeDeclFunction))
    {
        report(sema->reports, eFatal, decl->node, "extern attribute can only be applied to functions");
        return;
    }

    const tree_attribs_t *old = tree_get_attrib(decl);
    if (old->mangle != NULL)
    {
        report(sema->reports, eFatal, decl->node, "extern attribute already applied");
        return;
    }

    tree_attribs_t *copy = ctu_memdup(old, sizeof(tree_attribs_t));
    if (vector_len(args) == 0)
    {
        copy->mangle = tree_get_name(decl);
    }
    else
    {
        const char *msg = get_first_string(sema, decl, args);
        if (msg == NULL) { return; }

        copy->mangle = msg;
    }
    tree_set_attrib(decl, copy);
}

static void apply_layout(tree_t *sema, tree_t *decl, vector_t *args)
{
    if (!tree_is(decl, eTreeTypeStruct))
    {
        report(sema->reports, eFatal, decl->node, "layout attribute can only be applied to structs");
        return;
    }

    report(sema->reports, eWarn, tree_get_node(decl), "layout attribute not implemented");
}

void ctu_init_attribs(tree_t *sema)
{
    ctu_attrib_t *entry = attrib_create("entry", apply_entry);
    tree_module_set(sema, eCtuTagAttribs, entry->name, entry);

    ctu_attrib_t *deprecated = attrib_create("deprecated", apply_deprecated);
    tree_module_set(sema, eCtuTagAttribs, deprecated->name, deprecated);

    ctu_attrib_t *section = attrib_create("section", apply_section);
    tree_module_set(sema, eCtuTagAttribs, section->name, section);

    ctu_attrib_t *attribExtern = attrib_create("extern", apply_extern);
    tree_module_set(sema, eCtuTagAttribs, attribExtern->name, attribExtern);

    ctu_attrib_t *layout = attrib_create("layout", apply_layout);
    tree_module_set(sema, eCtuTagAttribs, layout->name, layout);
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
