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
            return tree_raise(type->node, sema->reports, "namespace `%s` not found", segment);
        }
    }

    const char *name = vector_tail(type->typeName);
    tree_t *decl = ctu_get_type(ns, name);
    if (decl == NULL)
    {
        return tree_raise(type->node, sema->reports, "type `%s` not found", name);
    }

    return decl;
}

static tree_t *ctu_sema_type_pointer(tree_t *sema, const ctu_t *type)
{
    tree_t *pointee = ctu_sema_type(sema, type->pointer);
    return tree_type_pointer(type->node, format("*%s", tree_get_name(pointee)), pointee, 1);
}

static tree_t *sema_type_function(tree_t *sema, const ctu_t *type)
{
    tree_t *returnType = ctu_sema_type(sema, type->returnType);

    size_t len = vector_len(type->params);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ctu_t *param = vector_get(type->params, i);
        tree_t *ty = ctu_sema_type(sema, param);
        if (tree_is(ty, eTreeTypeUnit))
        {
            report(sema->reports, eFatal, param->node, "void type is not allowed in function parameters");
        }

        tree_t *it = tree_decl_param(param->node, "", ty);
        vector_set(params, i, it);
    }

    return tree_type_closure(type->node, "", returnType, params, eArityFixed);
}

tree_t *ctu_sema_type(tree_t *sema, const ctu_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eCtuTypePointer: return ctu_sema_type_pointer(sema, type);
    case eCtuTypeName: return sema_type_name(sema, type);
    case eCtuTypeFunction: return sema_type_function(sema, type);

    default: NEVER("invalid type kind %d", type->kind);
    }
}
