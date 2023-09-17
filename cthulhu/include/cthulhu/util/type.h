#pragma once

#include <stdbool.h>

typedef struct tree_t tree_t;

///
/// type helpers
///

bool util_types_equal(const tree_t *lhs, const tree_t *rhs);

bool util_types_comparable(const tree_t *lhs, const tree_t *rhs);

/**
 * @brief attempt to convert the expr @arg expr to a type of @arg dst
 *
 * @param dst the desired type
 * @param expr the expression to try and cast
 * @return tree_t* the casted expression or @a tree_error if the cast could not be done
 */
tree_t *util_type_cast(const tree_t *dst, tree_t *expr);

///
/// length helpers
///

bool util_length_bounded(size_t length);
const char *util_length_name(size_t length);
