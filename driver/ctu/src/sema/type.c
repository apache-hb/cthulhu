#include "ctu/sema/type.h"
#include "ctu/sema/sema.h"
#include "ctu/ast.h"

#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/str.h"

#include "report/report.h"

#include "base/panic.h"

///
/// sema type
///

static tree_t *sema_type_name(tree_t *sema, const ctu_t *type)
{
    size_t len = vector_len(type->typeName);
    tree_t *ns = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *segment = vector_get(type->typeName, i);
        ns = ctu_get_namespace(ns, segment);
        if (ns == NULL)
        {
            report(sema->reports, eFatal, type->node, "namespace `%s` not found", segment);
            return tree_error(type->node, "namespace not found");
        }
    }

    const char *name = vector_tail(type->typeName);
    tree_t *decl = ctu_get_type(ns, name);
    if (decl == NULL)
    {
        report(sema->reports, eFatal, type->node, "type `%s` not found", name);
        return tree_error(type->node, "type not found");
    }

    return decl;
}

static tree_t *ctu_sema_type_pointer(tree_t *sema, const ctu_t *type)
{
    tree_t *pointee = ctu_sema_type(sema, type->pointer);
    return tree_type_pointer(type->node, format("*%s", tree_get_name(pointee)), pointee);
}

tree_t *ctu_sema_type(tree_t *sema, const ctu_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eCtuTypePointer: return ctu_sema_type_pointer(sema, type);
    case eCtuTypeName: return sema_type_name(sema, type);

    default: NEVER("invalid type kind %d", type->kind);
    }
}

///
/// query type
///

bool ctu_type_is(const tree_t *type, tree_kind_t kind)
{
    while (tree_is(type, eTreeQualify))
    {
        type = type->qualify;
    }

    return tree_is(type, kind);
}

///
/// format type
///

const char *ctu_type_string(const tree_t *type)
{
    return tree_to_string(type); // TODO: make this match cthulhu type signatures
}
