#pragma once

#include <stdbool.h>

typedef struct Hlir hlir_t;
typedef struct reports_t reports_t;

/**
 * @brief get a user readable string representation of an expression type
 *
 * @param type the expression to format
 * @param detail do we want a detailed name or a simple name
 * @return const char* the formatted name
 */
const char *ctu_repr(reports_t *reports, const hlir_t *type, bool detail);

/**
 * @brief get a user readable string representation of an type
 *
 * @param type the type to format
 * @param detail do we want a detailed name or a simple name
 * @return const char* the formatted name
 */
const char *ctu_type_repr(reports_t *reports, const hlir_t *type, bool detail);
