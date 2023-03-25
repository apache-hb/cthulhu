#include "common.h"

#include "base/panic.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include <stdio.h>

static bool has_name(hlir_kind_t kind)
{
    switch (kind)
    {
    case eHlirUnresolved:
    case eHlirStruct:
    case eHlirUnion:
    case eHlirDigit:
    case eHlirDecimal:
    case eHlirBool:
    case eHlirString:
    case eHlirEmpty:
    case eHlirUnit:
    case eHlirClosure:
    case eHlirPointer:
    case eHlirArray:
    case eHlirVaArgs:
    case eHlirVaList:

    case eHlirType:
    case eHlirAlias:
    case eHlirOpaque:

    case eHlirRecordField:
    case eHlirFunction:
    case eHlirQualified:

    case eHlirGlobal:
    case eHlirLocal:
    case eHlirParam:

    case eHlirError:

    case eHlirModule:
        return true;

    default:
        return false;
    }
}

static bool has_attribs(hlir_kind_t kind)
{
    switch (kind)
    {
    case eHlirUnresolved:
    case eHlirStruct:
    case eHlirUnion:
    case eHlirAlias:
    case eHlirDigit:
    case eHlirDecimal:
    case eHlirBool:
    case eHlirString:
    case eHlirPointer:
    case eHlirEmpty:
    case eHlirUnit:
    case eHlirClosure:
    case eHlirArray:
    case eHlirOpaque:
    case eHlirQualified:

    case eHlirRecordField:
    case eHlirFunction:

    case eHlirGlobal:
    case eHlirLocal:
    case eHlirParam:
        return true;

    default:
        return false;
    }
}

USE_DECL
hlir_kind_t get_hlir_kind(const hlir_t *hlir)
{
    CTASSERT(hlir != NULL);
    return hlir->type;
}

USE_DECL
const hlir_t *get_hlir_type(const hlir_t *hlir)
{
    CTASSERT(hlir != NULL);
    return hlir->of;
}

USE_DECL
const char *get_hlir_name(const hlir_t *hlir)
{
    CTASSERT(hlir != NULL);
    hlir_kind_t kind = get_hlir_kind(hlir);
    CTASSERTF(has_name(kind), "hlir_t %s has no name", hlir_kind_to_string(kind));

    return hlir->name;
}

const hlir_attributes_t *get_hlir_attributes(const hlir_t *hlir)
{
    CTASSERT(hlir != NULL);
    hlir_kind_t kind = get_hlir_kind(hlir);
    CTASSERTF(has_attribs(kind), "hlir %s has no attributes", hlir_kind_to_string(kind));

    return hlir->attributes;
}

node_t *get_hlir_node(const hlir_t *hlir)
{
    CTASSERT(hlir != NULL);
    return hlir->location;
}

bool hlir_is(const hlir_t *hlir, hlir_kind_t kind)
{
    return get_hlir_kind(hlir) == kind;
}

///
/// specific
///

bool hlir_is_type(const hlir_t *hlir)
{
    switch (get_hlir_kind(hlir))
    {
    case eHlirStruct:
    case eHlirUnion:
    case eHlirDigit:
    case eHlirBool:
    case eHlirString:
    case eHlirEmpty:
    case eHlirUnit:
    case eHlirType:
    case eHlirAlias:
        return true;

    default:
        return false;
    }
}

bool hlir_is_decl(const hlir_t *hlir)
{
    switch (get_hlir_kind(hlir))
    {
    case eHlirFunction:
    case eHlirGlobal:
        return true;

    default:
        return false;
    }
}

///
/// debugging
///

static const char *kKindNames[eHlirTotal] = {
#define HLIR_KIND(ID, STR) [ID] = (STR),
#include "cthulhu/hlir/hlir-def.inc"
};

static const char *kDigitNames[eDigitTotal] = {
#define DIGIT_KIND(ID, STR) [ID] = (STR),
#include "cthulhu/hlir/hlir-def.inc"
};

static const char *kSignNames[eSignTotal] = {
#define SIGN_KIND(ID, STR) [ID] = (STR),
#include "cthulhu/hlir/hlir-def.inc"
};

const char *hlir_kind_to_string(hlir_kind_t kind)
{
    return kKindNames[kind];
}

const char *hlir_sign_to_string(sign_t sign)
{
    return kSignNames[sign];
}

const char *hlir_digit_to_string(digit_t digit)
{
    return kDigitNames[digit];
}

const hlir_t *hlir_follow_type(const hlir_t *hlir)
{
    if (hlir_is(hlir, eHlirAlias))
    {
        return hlir_follow_type(hlir->alias);
    }

    if (hlir_is(hlir, eHlirParam) || hlir_is(hlir, eHlirQualified))
    {
        return hlir_follow_type(get_hlir_type(hlir));
    }

    return hlir;
}

const hlir_t *hlir_real_type(const hlir_t *hlir)
{
    if (hlir_is(hlir, eHlirAlias))
    {
        return hlir_follow_type(hlir->alias);
    }

    if (hlir_is(hlir, eHlirParam))
    {
        return hlir_follow_type(get_hlir_type(hlir));
    }

    return hlir;
}

bool hlir_types_equal(const hlir_t *lhs, const hlir_t *rhs)
{
    const hlir_t *actualLhs = hlir_follow_type(lhs);
    const hlir_t *actualRhs = hlir_follow_type(rhs);

    if ((lhs == rhs) || (actualLhs == actualRhs))
    {
        return true;
    }

    hlir_kind_t lhsKind = get_hlir_kind(actualLhs);
    hlir_kind_t rhsKind = get_hlir_kind(actualRhs);
    if (hlir_is_callable(actualLhs) && hlir_is_callable(actualRhs))
    {
        // TODO: this is kinda ugly   
    }
    else if (lhsKind != rhsKind)
    {
        return false;
    }

    switch (lhsKind)
    {
    case eHlirDigit:
        return actualLhs->width == actualRhs->width && actualLhs->sign == actualRhs->sign;
    case eHlirString: // TODO: update this when we have multiple string encodings
    case eHlirBool:
    case eHlirEmpty:
    case eHlirUnit:
        return true;

    case eHlirPointer:
        return actualLhs->indexable == actualRhs->indexable && hlir_types_equal(actualLhs->ptr, actualRhs->ptr);

    case eHlirClosure:
    case eHlirFunction: {
        vector_t *lhsParams = closure_params(actualLhs);
        vector_t *rhsParams = closure_params(actualRhs);
        if (vector_len(lhsParams) != vector_len(rhsParams))
        {
            return false;
        }

        if (closure_variadic(actualLhs) != closure_variadic(actualRhs))
        {
            return false;
        }

        for (size_t i = 0; i < vector_len(lhsParams); i++)
        {
            const hlir_t *lhsParam = vector_get(lhsParams, i);
            const hlir_t *rhsParam = vector_get(rhsParams, i);
            if (!hlir_types_equal(lhsParam, rhsParam))
            {
                return false;
            }
        }

        return true;
    }

    case eHlirStruct:
    case eHlirUnion: {
        vector_t *lhsFields = actualLhs->fields;
        vector_t *rhsFields = actualLhs->fields;
        if (vector_len(lhsFields) != vector_len(rhsFields))
        {
            return false;
        }

        for (size_t i = 0; i < vector_len(lhsFields); i++)
        {
            const hlir_t *lhsField = vector_get(lhsFields, i);
            const hlir_t *rhsField = vector_get(rhsFields, i);
            if (!hlir_types_equal(lhsField, rhsField))
            {
                return false;
            }
        }

        return true;
    }

    case eHlirAlias:
    case eHlirError: // TODO: are errors always equal, or never equal?
        return false;

    default:
        CTASSERTF(false, "unknown type %s", hlir_kind_to_string(lhsKind));
        return false;
    }
}
