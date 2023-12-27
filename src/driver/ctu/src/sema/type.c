#include "ctu/sema/type.h"
#include "cthulhu/events/events.h"
#include "ctu/sema/sema.h"
#include "ctu/sema/expr.h"
#include "ctu/ast.h"

#include "cthulhu/util/util.h"

#include "cthulhu/tree/query.h"

#include "std/vector.h"
#include "std/str.h"

#include "base/panic.h"

///
/// sema type
///

static const size_t kLocalModuleTags[] = { eCtuTagModules };
static const size_t kGlobalModuleTags[] = { eCtuTagImports };
static const size_t kDeclTags[] = { eCtuTagTypes };

static const decl_search_t kSearchType = {
    .local_tags = kLocalModuleTags,
    .local_count = sizeof(kLocalModuleTags) / sizeof(size_t),

    .global_tags = kGlobalModuleTags,
    .global_count = sizeof(kGlobalModuleTags) / sizeof(size_t),

    .decl_tags = kDeclTags,
    .decl_count = sizeof(kDeclTags) / sizeof(size_t)
};

static tree_t *sema_type_name(tree_t *sema, const ctu_t *type)
{
    return util_search_path(sema, &kSearchType, type->node, type->typeName);
}

static tree_t *ctu_sema_type_pointer(ctu_sema_t *sema, const ctu_t *type)
{
    tree_t *pointee = ctu_sema_type(sema, type->pointer);
    return tree_type_pointer(type->node, format("*%s", tree_get_name(pointee)), pointee, 1);
}

static tree_t *sema_type_function(ctu_sema_t *sema, const ctu_t *type)
{
    tree_t *result = ctu_sema_type(sema, type->returnType);

    size_t len = vector_len(type->params);
    vector_t *params = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        const ctu_t *param = vector_get(type->params, i);
        tree_t *ty = ctu_sema_type(sema, param);
        if (tree_is(ty, eTreeTypeUnit))
        {
            msg_notify(ctu_sema_reports(sema), &kEvent_InvalidFunctionSignature, param->node, "void type is not allowed in function parameters");
        }

        tree_t *it = tree_decl_param(param->node, "", ty);
        vector_set(params, i, it);
    }

    return tree_type_closure(type->node, "", result, params, eArityFixed);
}

static tree_t *sema_type_array(ctu_sema_t *sema, const ctu_t *type)
{
    tree_t *inner = ctu_sema_type(sema, type->arrayType);
    tree_t *length = ctu_sema_rvalue(sema, type->arrayLength, ctu_get_int_type(eDigitSize, eSignUnsigned));

    mpz_t value;
    mpz_init(value);
    if (!util_eval_digit(value, length))
    {
        msg_notify(ctu_sema_reports(sema), &kEvent_InvalidArraySize, type->node, "array length must be a constant integer");
    }

    size_t v = mpz_get_ui(value);

    return tree_type_array(type->node, "", inner, v);
}

tree_t *ctu_sema_type(ctu_sema_t *sema, const ctu_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eCtuTypePointer: return ctu_sema_type_pointer(sema, type);
    case eCtuTypeName: return sema_type_name(sema->sema, type);
    case eCtuTypeFunction: return sema_type_function(sema, type);
    case eCtuTypeArray: return sema_type_array(sema, type);

    default: NEVER("invalid type kind %d", type->kind);
    }
}
