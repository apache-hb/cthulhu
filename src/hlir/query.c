#include "common.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"

#if ENABLE_DEBUG

static bool has_name(hlir_kind_t kind)
{
    switch (kind)
    {
    case HLIR_STRUCT:
    case HLIR_UNION:
    case HLIR_DIGIT:
    case HLIR_BOOL:
    case HLIR_STRING:
    case HLIR_VOID:
    case HLIR_CLOSURE:

        // unsure about closure, pointer, and array

    case HLIR_TYPE:
    case HLIR_ALIAS:

    case HLIR_FORWARD:
    case HLIR_FIELD:
    case HLIR_FUNCTION:
    case HLIR_GLOBAL:
    case HLIR_LOCAL:

    case HLIR_MODULE:
        return true;

    default:
        return false;
    }
}

static bool has_attribs(hlir_kind_t kind)
{
    switch (kind)
    {
    case HLIR_STRUCT:
    case HLIR_UNION:
    case HLIR_ALIAS:
    case HLIR_DIGIT:
    case HLIR_BOOL:
    case HLIR_STRING:
    case HLIR_POINTER:
    case HLIR_VOID:
    case HLIR_CLOSURE:

    case HLIR_FIELD:
    case HLIR_FORWARD:
    case HLIR_FUNCTION:
    case HLIR_GLOBAL:
    case HLIR_LOCAL:
        return true;

    default:
        return false;
    }
}

#endif

hlir_kind_t get_hlir_kind(const hlir_t *hlir)
{
    CHECK_NULL(hlir);
    return hlir->type;
}

const hlir_t *get_hlir_type(const hlir_t *hlir)
{
    CHECK_NULL(hlir);
    return hlir->of;
}

const char *get_hlir_name(const hlir_t *hlir)
{
#if ENABLE_DEBUG
    CHECK_NULL(hlir);
    hlir_kind_t kind = get_hlir_kind(hlir);
    CTASSERTF(has_name(kind), "hlir_t %s has no name", hlir_kind_to_string(kind));
#endif

    return hlir->name;
}

const hlir_attributes_t *get_hlir_attributes(const hlir_t *hlir)
{
#if ENABLE_DEBUG
    CHECK_NULL(hlir);
    hlir_kind_t kind = get_hlir_kind(hlir);
    CTASSERTF(has_attribs(kind), "hlir_t %s has no attributes", hlir_kind_to_string(kind));
#endif

    return hlir->attributes;
}

const node_t *get_hlir_node(const hlir_t *hlir)
{
    CHECK_NULL(hlir);
    return hlir->location;
}

const hlir_t *get_hlir_parent(const hlir_t *hlir)
{
    CHECK_NULL(hlir);
    return hlir->parentDecl;
}

bool hlir_is(const hlir_t *hlir, hlir_kind_t kind)
{
    return get_hlir_kind(hlir) == kind;
}

bool hlir_will_be(const hlir_t *hlir, hlir_kind_t kind)
{
    return hlir_is(hlir, HLIR_FORWARD) && hlir->expected == kind;
}

bool hlis_is_or_will_be(const hlir_t *hlir, hlir_kind_t kind)
{
    return hlir_is(hlir, kind) || hlir_will_be(hlir, kind);
}

///
/// specific
///

bool hlir_is_type(const hlir_t *hlir)
{
    switch (get_hlir_kind(hlir))
    {
    case HLIR_STRUCT:
    case HLIR_UNION:
    case HLIR_DIGIT:
    case HLIR_BOOL:
    case HLIR_STRING:
    case HLIR_VOID:
    case HLIR_TYPE:
    case HLIR_ALIAS:
        return true;

    default:
        return false;
    }
}

bool hlir_is_decl(const hlir_t *hlir)
{
    switch (get_hlir_kind(hlir))
    {
    case HLIR_FUNCTION:
    case HLIR_GLOBAL:
        return true;

    default:
        return false;
    }
}

///
/// debugging
///

static const char *kKindNames[HLIR_TOTAL] = {
    [HLIR_DIGIT_LITERAL] = "digit-literal",
    [HLIR_BOOL_LITERAL] = "bool-literal",
    [HLIR_STRING_LITERAL] = "string-literal",

    [HLIR_NAME] = "name",
    [HLIR_UNARY] = "unary",
    [HLIR_BINARY] = "binary",
    [HLIR_COMPARE] = "compare",
    [HLIR_CALL] = "call",

    [HLIR_STMTS] = "stmt-list",
    [HLIR_BRANCH] = "branch",
    [HLIR_LOOP] = "loop",
    [HLIR_ASSIGN] = "assign",

    [HLIR_STRUCT] = "struct-type",
    [HLIR_UNION] = "union-type",
    [HLIR_DIGIT] = "digit-type",
    [HLIR_BOOL] = "bool-type",
    [HLIR_STRING] = "string-type",
    [HLIR_VOID] = "void-type",
    [HLIR_CLOSURE] = "closure-type",
    [HLIR_POINTER] = "pointer-type",
    [HLIR_ARRAY] = "array-type",
    [HLIR_TYPE] = "metatype",
    [HLIR_ALIAS] = "alias-type",

    [HLIR_LOCAL] = "local",
    [HLIR_PARAM] = "param",
    [HLIR_GLOBAL] = "global",

    [HLIR_FORWARD] = "forward",
    [HLIR_FUNCTION] = "function",
    [HLIR_MODULE] = "module",

    [HLIR_FIELD] = "field",

    [HLIR_ERROR] = "internal-error",
};

static const char *kDigitNames[DIGIT_TOTAL] = {
    [DIGIT_CHAR] = "char", [DIGIT_SHORT] = "short", [DIGIT_INT] = "int",
    [DIGIT_LONG] = "long", [DIGIT_SIZE] = "size",   [DIGIT_PTR] = "intptr",
};

static const char *kSignNames[SIGN_TOTAL] = {[SIGN_SIGNED] = "signed", [SIGN_UNSIGNED] = "unsigned"};

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
