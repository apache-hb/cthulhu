#include "attribs.h"
#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/hlir.h"
#include "sema.h"

#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"
#include "base/util.h"

#include "std/map.h"
#include "std/str.h"

#include "cthulhu/hlir/sema.h"

#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/query.h"

#include <stdio.h>

static attrib_t *attrib_new(const char *name, hlir_kind_t expected, apply_attribs_t apply, apply_accepts_t accepts)
{
    attrib_t *attrib = ctu_malloc(sizeof(attrib_t));
    attrib->name = name;
    attrib->expectedKind = expected;
    attrib->apply = apply;
    attrib->accepts = accepts;
    return attrib;
}

static hlir_attributes_t *apply_entry(sema_t *sema, hlir_t *hlir, ast_t *ast)
{
    linkage_t linkage = eLinkEntryCli;
    if (vector_len(ast->config) > 1)
    {
        report(sema_reports(sema), eFatal, ast->node, "entry only takes 0 or 1 parameters");
        return NULL;
    }

    if (vector_len(ast->config) == 1)
    {
        ast_t *entry = vector_get(ast->config, 0);
        if (entry->of != eAstName)
        {
            report(sema_reports(sema), eFatal, entry->node, "entry param must be either 'cli' or 'gui'");
            return NULL;
        }

        if (vector_len(entry->path) != 1)
        {
            report(sema_reports(sema), eFatal, entry->node, "entry param must be either 'cli' or 'gui'");
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
            report(sema_reports(sema), eFatal, entry->node, "entry param must be either 'cli' or 'gui'");
            return NULL;
        }
    }

    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    if (attribs->linkage == eLinkImported)
    {
        report(sema_reports(sema), eFatal, get_hlir_node(hlir), "entry point cannot be imported");
        return NULL;
    }

    if (is_entry_point(attribs->linkage))
    {
        report(sema_reports(sema), eFatal, ast->node, "overriding entry point attribute");
        return NULL;
    }

    hlir_attributes_t *newAttributes = ctu_memdup(attribs, sizeof(hlir_attributes_t));

    newAttributes->linkage = linkage;

    return newAttributes;
}

static bool accept_entry(hlir_t *hlir)
{
    return hlir_is(hlir, eHlirFunction);
}

static hlir_attributes_t *apply_extern(sema_t *sema, hlir_t *hlir, ast_t *ast)
{
    if (vector_len(ast->config) > 1)
    {
        report(sema_reports(sema), eFatal, ast->node, "attribute takes up to 1 parameter");
        return NULL;
    }

    const char *mangle = get_hlir_name(hlir);
    if (vector_len(ast->config) == 1)
    {
        ast_t *entry = vector_get(ast->config, 0);
        if (entry->of != eAstString)
        {
            report(sema_reports(sema), eFatal, entry->node, "extern argument must be a string");
            return NULL;
        }
        mangle = ctu_strndup(entry->string, entry->length);
    }

    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);
    hlir_attributes_t *newAttributes = ctu_memdup(attribs, sizeof(hlir_attributes_t));

    newAttributes->mangle = mangle;
    newAttributes->linkage = eLinkImported;

    return newAttributes;
}

static bool accept_extern(hlir_t *hlir)
{
    return hlir_is(hlir, eHlirFunction) || hlir_is(hlir, eHlirGlobal);
}

static hlir_attributes_t *apply_layout(sema_t *sema, hlir_t *hlir, ast_t *ast)
{
    UNUSED(ast);
    report(sema_reports(sema), eWarn, get_hlir_node(hlir), "layout not implemented");
    return NULL;
}

static bool accept_layout(hlir_t *hlir)
{
    return hlir_is(hlir, eHlirStruct);
}

static void add_attrib(sema_t *sema, const char *name, hlir_kind_t kind, apply_attribs_t apply, apply_accepts_t accepts)
{
    sema_set(sema, eTagAttribs, name, attrib_new(name, kind, apply, accepts));
}

void add_builtin_attribs(sema_t *sema)
{
    add_attrib(sema, "entry", eHlirFunction, apply_entry, accept_entry);
    add_attrib(sema, "extern", eHlirFunction, apply_extern, accept_extern);
    add_attrib(sema, "layout", eHlirStruct, apply_layout, accept_layout);
}

static const char *kDeclNames[eHlirTotal] = {
    [eHlirStruct] = "struct",     
    [eHlirUnion] = "union",         
    [eHlirAlias] = "type alias",
    [eHlirFunction] = "function", 
    [eHlirGlobal] = "global value", 
    [eHlirRecordField] = "field",
    [eHlirError] = "error"
};

static const char *get_pretty_decl_name(hlir_t *hlir)
{
    return kDeclNames[get_hlir_kind(hlir)];
}

static void apply_single_attrib(sema_t *sema, hlir_t *hlir, ast_t *attr)
{
    attrib_t *attrib = sema_get(sema, eTagAttribs, attr->name);
    printf("attr: %s %p\n", attr->name, (void*)attrib);
    if (attrib == NULL)
    {
        report(sema_reports(sema), eWarn, attr->node, "unknown attribute '%s'", attr->name);
        return;
    }

    printf("accept: %p\n", attrib->accepts);

    if (!attrib->accepts(hlir))
    {
        report(sema_reports(sema), eFatal, attr->node, "attribute '%s' is for %ss, was provided with a %s instead",
               attrib->name, 
               kDeclNames[attrib->expectedKind], 
               get_pretty_decl_name(hlir));
        return;
    }

    hlir_attributes_t *newAttribs = attrib->apply(sema, hlir, attr);
    if (newAttribs == NULL)
    {
        return;
    }

    hlir_set_attributes(hlir, newAttribs);
}

void apply_attributes(sema_t *sema, hlir_t *hlir, ast_t *ast)
{
    CTASSERT(hlir != NULL);

    size_t totalAttribs = vector_len(ast->attribs);
    for (size_t i = 0; i < totalAttribs; i++)
    {
        ast_t *attrib = vector_get(ast->attribs, i);
        apply_single_attrib(sema, hlir, attrib);
    }
}
