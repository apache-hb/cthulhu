#include "attribs.h"
#include "sema.h"

#include "report/report.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"
#include "base/util.h"

#include "std/map.h"
#include "std/str.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"

static attrib_t *attrib_new(const char *name, h2_kind_t expected, apply_attribs_t apply, apply_accepts_t accepts)
{
    attrib_t *attrib = ctu_malloc(sizeof(attrib_t));
    attrib->name = name;
    attrib->expectedKind = expected;
    attrib->apply = apply;
    attrib->accepts = accepts;
    return attrib;
}

static bool is_entry_point(h2_link_t linkage)
{
    return linkage == eLinkEntryCli || linkage == eLinkEntryGui;
}

static h2_attrib_t *apply_entry(h2_t *sema, h2_t *hlir, ast_t *ast)
{
    h2_link_t linkage = eLinkEntryCli;
    if (vector_len(ast->config) > 1)
    {
        report(sema->reports, eFatal, ast->node, "entry only takes 0 or 1 parameters");
        return NULL;
    }

    if (vector_len(ast->config) == 1)
    {
        ast_t *entry = vector_get(ast->config, 0);
        if (entry->of != eAstName)
        {
            report(sema->reports, eFatal, entry->node, "entry param must be either 'cli' or 'gui'");
            return NULL;
        }

        if (vector_len(entry->path) != 1)
        {
            report(sema->reports, eFatal, entry->node, "entry param must be either 'cli' or 'gui'");
            return NULL;
        }

        const char *name = vector_get(entry->path, 0);
        if (str_equal(name, "cli"))
        {
            linkage = eLinkEntryCli;
        }
        else if (str_equal(name, "gui"))
        {
            linkage = eLinkEntryGui;
        }
        else
        {
            report(sema->reports, eFatal, entry->node, "entry param must be either 'cli' or 'gui'");
            return NULL;
        }
    }

    const h2_attrib_t *attribs = h2_get_attrib(hlir);
    if (attribs->link == eLinkImport)
    {
        report(sema->reports, eFatal, h2_get_node(hlir), "entry point cannot be imported");
        return NULL;
    }

    if (is_entry_point(attribs->link))
    {
        report(sema->reports, eFatal, ast->node, "overriding entry point attribute");
        return NULL;
    }

    // TODO: weird
    h2_attrib_t *newAttributes = ctu_memdup(attribs, sizeof(h2_attrib_t));

    newAttributes->link = linkage;

    return newAttributes;
}

static bool accept_entry(h2_t *hlir)
{
    return h2_is(hlir, eHlir2DeclFunction);
}

static h2_attrib_t *apply_extern(sema_t *sema, h2_t *hlir, ast_t *ast)
{
    if (vector_len(ast->config) > 1)
    {
        report(sema->reports, eFatal, ast->node, "attribute takes up to 1 parameter");
        return NULL;
    }

    const char *mangle = h2_get_name(hlir);
    if (vector_len(ast->config) == 1)
    {
        ast_t *entry = vector_get(ast->config, 0);
        if (entry->of != eAstString)
        {
            report(sema->reports, eFatal, entry->node, "extern argument must be a string");
            return NULL;
        }
        mangle = ctu_strndup(entry->string, entry->length);
    }

    const h2_attrib_t *attribs = h2_get_attrib(hlir);
    h2_attrib_t *newAttributes = ctu_memdup(attribs, sizeof(h2_attrib_t));

    newAttributes->mangle = mangle;
    newAttributes->link = eLinkImport;

    return newAttributes;
}

static bool accept_extern(h2_t *hlir)
{
    return h2_is(hlir, eHlir2DeclFunction) || h2_is(hlir, eHlir2DeclGlobal);
}

static h2_attrib_t *apply_layout(sema_t *sema, h2_t *hlir, ast_t *ast)
{
    CTU_UNUSED(ast);
    report(sema->reports, eWarn, h2_get_node(hlir), "layout not implemented");
    return NULL;
}

static bool accept_layout(h2_t *hlir)
{
    return h2_is(hlir, eHlir2TypeStruct);
}

static void add_attrib(sema_t *sema, const char *name, h2_kind_t kind, apply_attribs_t apply, apply_accepts_t accepts)
{
    h2_module_set(sema, eTagAttribs, name, attrib_new(name, kind, apply, accepts));
}

void add_builtin_attribs(sema_t *sema)
{
    add_attrib(sema, "entry", eHlir2DeclFunction, apply_entry, accept_entry);
    add_attrib(sema, "extern", eHlir2DeclFunction, apply_extern, accept_extern);
    add_attrib(sema, "layout", eHlir2TypeStruct, apply_layout, accept_layout);
}

static const char *kDeclNames[eHlir2Total] = {
    [eHlir2TypeStruct] = "struct",
    [eHlir2TypeUnion] = "union",
    [eHlir2TypeAlias] = "type alias",
    [eHlir2DeclFunction] = "function",
    [eHlir2DeclGlobal] = "global value",
    [eHlir2RecordField] = "field",
    [eHlir2Error] = "error"
};

static const char *get_pretty_decl_name(h2_t *hlir)
{
    return kDeclNames[h2_get_kind(hlir)];
}

static void apply_single_attrib(sema_t *sema, h2_t *hlir, ast_t *attr)
{
    attrib_t *attrib = h2_module_get(sema, eTagAttribs, attr->name);
    if (attrib == NULL)
    {
        report(sema->reports, eWarn, attr->node, "unknown attribute '%s'", attr->name);
        return;
    }

    if (!attrib->accepts(hlir))
    {
        report(sema->reports, eFatal, attr->node, "attribute '%s' is for %ss, was provided with a %s instead",
               attrib->name,
               kDeclNames[attrib->expectedKind],
               get_pretty_decl_name(hlir));
        return;
    }

    h2_attrib_t *newAttribs = attrib->apply(sema, hlir, attr);
    if (newAttribs == NULL)
    {
        return;
    }

    h2_set_attrib(hlir, newAttribs);
}

void apply_attributes(sema_t *sema, h2_t *hlir, ast_t *ast)
{
    CTASSERT(hlir != NULL);

    size_t totalAttribs = vector_len(ast->attribs);
    for (size_t i = 0; i < totalAttribs; i++)
    {
        ast_t *attrib = vector_get(ast->attribs, i);
        apply_single_attrib(sema, hlir, attrib);
    }
}
