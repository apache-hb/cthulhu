#pragma once

#include "ctu/type/type.h"
#include "ctu/lir/lir.h"

/**
 * retype untyped literals 
 *
 * @param reports the report sink
 * @param type the default type to be retyped to
 * @param expr the expr to retype
 *
 * @return the retyped expression tree
 */
lir_t *retype_expr(reports_t *reports, const type_t *type, lir_t *expr);
