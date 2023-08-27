#pragma once

#include <stdbool.h>

#include "cthulhu/tree/ops.h"

typedef struct node_t node_t;
typedef struct tree_t tree_t;

typedef struct util_digit_t {
    digit_t digit;
    sign_t sign;
    const char *name;
} util_digit_t;

typedef struct util_config_t {
    const char *langName;

    const char *unitName;
    const char *boolName;
    const char *stringName;

    const util_digit_t *digits;
    size_t totalDigits;
} util_config_t;

typedef struct util_types_t {
    const char *langName;

    tree_t *unitType;
    tree_t *boolType;
    tree_t *stringType;

    tree_t *digitTypes[eDigitTotal * eSignTotal];
} util_types_t;

///
/// util types
///

util_types_t util_base_create(util_config_t config, tree_t *sema);

tree_t *util_get_unit(util_types_t types);
tree_t *util_get_bool(util_types_t types);
tree_t *util_get_string(util_types_t types);
tree_t *util_get_digit(util_types_t types, digit_t digit, sign_t sign);

///
/// util query
///

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name);

bool util_types_equal(const tree_t *lhs, const tree_t *rhs);
