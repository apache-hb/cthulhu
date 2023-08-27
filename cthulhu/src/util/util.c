#include "cthulhu/util/util.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "scan/node.h"

#include "base/panic.h"

static size_t digit_index(digit_t digit, sign_t sign)
{
    return digit * eSignTotal + sign;
}

static void add_type(tree_t *sema, const char *name, tree_t *tree)
{
    tree_t *old = tree_module_set(sema, eSema2Types, name, tree);
    CTASSERTF(old == NULL, "type(%s) already exists", name);
}

static tree_t *create_type(tree_t *sema, const char *name, tree_t *type)
{
    if (name != NULL)
    {
        add_type(sema, name, type);
    }

    return type;
}

// TODO: a little ugly

#define CREATE_TYPE(ID, EXPR) ((ID) == NULL) ? NULL : create_type(sema, ID, EXPR(node_builtin(), ID))

util_types_t util_base_create(util_config_t config, tree_t *sema)
{
    node_t *node = node_builtin();
    util_types_t types = {
        .langName = config.langName,

        .unitType = CREATE_TYPE(config.unitName, tree_type_unit),
        .boolType = CREATE_TYPE(config.boolName, tree_type_bool),
        .stringType = CREATE_TYPE(config.stringName, tree_type_string),
    };

    for (size_t i = 0; i < config.totalDigits; i++)
    {
        util_digit_t digit = config.digits[i];
        tree_t *type = tree_type_digit(node, digit.name, digit.digit, digit.sign);

        types.digitTypes[digit_index(digit.digit, digit.sign)] = type;
        add_type(sema, digit.name, type);
    }

    return types;
}

tree_t *util_get_unit(util_types_t types)
{
    CTASSERTF(types.unitType != NULL, "unit type not initialized for %s", types.langName);
    return types.unitType;
}

tree_t *util_get_bool(util_types_t types)
{
    CTASSERTF(types.boolType != NULL, "bool type not initialized for %s", types.langName);
    return types.boolType;
}

tree_t *util_get_string(util_types_t types)
{
    CTASSERTF(types.stringType != NULL, "string type not initialized for %s", types.langName);
    return types.stringType;
}

tree_t *util_get_digit(util_types_t types, digit_t digit, sign_t sign)
{
    CTASSERTF(digit < eDigitTotal, "digit(%s) out of range", digit_name(digit));
    CTASSERTF(sign < eSignTotal, "sign(%s) out of range", sign_name(sign));

    CTASSERTF(types.digitTypes[digit_index(digit, sign)] != NULL, "digit(digit=%s, sign=%s) not initialized for %s", digit_name(digit), sign_name(sign), types.langName);
    return types.digitTypes[digit_index(digit, sign)];
}

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name)
{
    for (size_t i = 0; i < len; i++)
    {
        tree_t *decl = tree_module_get(sema, tags[i], name);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}
