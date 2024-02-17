#include "oberon/sema/type.h"

#include "oberon/driver.h"
#include "oberon/sema/expr.h"
#include "oberon/sema/sema.h"

#include "cthulhu/events/events.h"
#include "cthulhu/util/types.h"
#include "cthulhu/util/util.h"

#include "std/vector.h"

#include "memory/memory.h"
#include "base/panic.h"

static const size_t kModuleTags[] = { eObrTagModules, eObrTagImports };
static const size_t kDeclTags[] = { eObrTagTypes };

static const decl_search_t kSearchDecl = {
    .module_tags = kModuleTags,
    .module_count = sizeof(kModuleTags) / sizeof(size_t),

    .decl_tags = kDeclTags,
    .decl_count = sizeof(kDeclTags) / sizeof(size_t)
};

static tree_t *sema_type_name(tree_t *sema, obr_t *type)
{
    tree_t *it = obr_get_type(sema, type->name);
    if (it == NULL)
    {
        return tree_raise(type->node, sema->reports, &kEvent_TypeNotFound, "type '%s' not found", type->name);
    }

    return it;
}

static tree_t *sema_type_qual(tree_t *sema, obr_t *type)
{
    return util_search_qualified(sema, &kSearchDecl, type->node, type->name, type->symbol);
}

static tree_t *sema_type_pointer(tree_t *sema, obr_t *type, const char *name)
{
    tree_t *it = obr_sema_type(sema, type->pointer, name);
    return tree_type_pointer(type->node, name, it, SIZE_MAX);
}

static size_t sema_array_length(tree_t *sema, obr_t *expr)
{
    CTASSERT(sema != NULL);
    if (expr == NULL) return SIZE_MAX;

    tree_t *it = obr_sema_rvalue(sema, expr, obr_get_integer_type());
    if (tree_is(it, eTreeExprDigit))
    {
        int sign = mpz_sgn(it->digit_value);
        if (sign < 0)
        {
            msg_notify(sema->reports, &kEvent_ArrayLengthNegative, it->node, "array length cannot be negative");
            return SIZE_MAX;
        }
        else if (sign == 0)
        {
            msg_notify(sema->reports, &kEvent_ArrayLengthZero, it->node, "array length cannot be zero");
            return SIZE_MAX;
        }
        else if (mpz_fits_uint_p(it->digit_value) == 0)
        {
            msg_notify(sema->reports, &kEvent_ArrayLengthOverflow, it->node, "`%s` is too large to be an array length", mpz_get_str(NULL, 10, it->digit_value));
            return SIZE_MAX;
        }

        return mpz_get_ui(it->digit_value);
    }

    msg_notify(sema->reports, &kEvent_ArrayLengthNotConstant, it->node, "array length must be a constant");
    return SIZE_MAX;
}

static tree_t *sema_array_segment(tree_t *sema, obr_t *size, tree_t *element, const char *name)
{
    // even if get_array_length fails we still want to return a valid type
    size_t len = sema_array_length(sema, size);

    return tree_type_array(size->node, name, element, len);
}

static tree_t *sema_type_array(tree_t *sema, obr_t *type, const char *name)
{
    tree_t *it = obr_sema_type(sema, type->array_element, name); // TODO: will the name clash matter?

    size_t len = vector_len(type->sizes);

    // if theres no sizes its an open array
    if (len == 0)
        return tree_type_array(type->node, name, it, SIZE_MAX);

    // otherwise we need to iterate backwards over the sizes
    // to build up the array type
    for (size_t i = len; i > 0; i--)
    {
        obr_t *size = vector_get(type->sizes, i - 1);
        it = sema_array_segment(sema, size, it, name);
    }

    return it;
}

static tree_t *sema_type_record(tree_t *sema, obr_t *type, const char *name)
{
    size_t len = vector_len(type->fields);
    arena_t *arena = get_global_arena();
    vector_t *result = vector_of(len, arena);
    for (size_t i = 0; i < len; i++)
    {
        obr_t *field = vector_get(type->fields, i);
        tree_t *it = obr_sema_type(sema, field->type, field->name);
        tree_t *decl = tree_decl_field(field->node, field->name, it);
        vector_set(result, i, decl);
    }

    return tree_decl_struct(type->node, name, result);
}

tree_t *obr_sema_type(tree_t *sema, obr_t *type, const char *name)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eObrTypeName: return sema_type_name(sema, type);
    case eObrTypeQual: return sema_type_qual(sema, type);
    case eObrTypePointer: return sema_type_pointer(sema, type, name);
    case eObrTypeArray: return sema_type_array(sema, type, name);
    case eObrTypeRecord: return sema_type_record(sema, type, name);

    default: NEVER("unknown type kind %d", type->kind);
    }
}


///
/// query
///

const tree_t *obr_rvalue_type(const tree_t *self)
{
    return tree_ty_load_type(tree_get_type(self));
}


///
/// casting
///

static bool is_open_array(const tree_t *type)
{
    return tree_is(type, eTreeTypeArray) && !util_length_bounded(type->length);
}

tree_t *obr_cast_type(tree_t *expr, const tree_t *type)
{
    const tree_t *exprtype = tree_get_type(expr);

    // pointers can convert to open arrays and vice versa
    if ((is_open_array(exprtype) && tree_is(type, eTreeTypePointer))
        || (is_open_array(type) && tree_is(exprtype, eTreeTypePointer))
    )
    {
        if (util_types_equal(exprtype->ptr, type->ptr))
        {
            return tree_expr_cast(expr->node, type, expr);
        }
    }

    if (tree_is(exprtype, eTreeTypeArray) && tree_is(type, eTreeTypeArray))
    {
        if (exprtype->length >= type->length || !util_length_bounded(exprtype->length) || !util_length_bounded(type->length))
        {
            if (util_types_equal(exprtype->ptr, type->ptr))
            {
                return tree_expr_cast(expr->node, type, expr);
            }
        }
    }

    // TODO: deal with other casts
    return expr;
}
