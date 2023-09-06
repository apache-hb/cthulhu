#pragma once

#include <stdbool.h>

#include <gmp.h>

typedef struct tree_t tree_t;

///
/// query helpers
///

void *util_select_decl(tree_t *sema, const size_t *tags, size_t len, const char *name);

///
/// context
///

tree_t *util_current_module(tree_t *sema);
void util_set_current_module(tree_t *sema, tree_t *module);

tree_t *util_current_symbol(tree_t *sema);
void util_set_current_symbol(tree_t *sema, tree_t *symbol);

///
/// type helpers
///

bool util_types_equal(const tree_t *lhs, const tree_t *rhs);

/**
 * @brief attempt to convert the expr @arg expr to a type of @arg dst
 *
 * @param dst the desired type
 * @param expr the expression to try and cast
 * @return tree_t* the casted expression or @a tree_error if the cast could not be done
 */
tree_t *util_type_cast(const tree_t *dst, tree_t *expr);

///
/// eval
///

bool util_eval_digit(mpz_t value, const tree_t *expr);

///
/// string helpers
///

tree_t *util_create_string(tree_t *sema, tree_t *letter, const char *text, size_t length);

///
/// length helpers
///

bool util_length_bounded(size_t length);
const char *util_length_name(size_t length);
