// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include "core/text.h"
#include "cthulhu/ssa/ssa.h"

///
/// type api
///

ssa_type_t *ssa_type_new(ssa_kind_t kind, const char *name, tree_quals_t quals);

ssa_type_t *ssa_type_empty(const char *name, tree_quals_t quals);
ssa_type_t *ssa_type_unit(const char *name, tree_quals_t quals);
ssa_type_t *ssa_type_closure(const char *name, tree_quals_t quals, ssa_type_t *result, typevec_t *params, bool variadic);
ssa_type_t *ssa_type_array(const char *name, tree_quals_t quals, ssa_type_t *element, size_t length);
ssa_type_t *ssa_type_struct(const char *name, tree_quals_t quals, typevec_t *fields);
ssa_type_t *ssa_type_union(const char *name, tree_quals_t quals, typevec_t *fields);

ssa_type_t *ssa_type_create_cached(map_t *cache, const tree_t *type);

ssa_type_t *ssa_type_common(const ssa_type_t *lhs, const ssa_type_t *rhs);

///
/// value api
///

ssa_value_t *ssa_value_empty(const ssa_type_t *type);
ssa_value_t *ssa_value_unit(const ssa_type_t *type);
ssa_value_t *ssa_value_bool(const ssa_type_t *type, bool value);
ssa_value_t *ssa_value_digit(const ssa_type_t *type, const mpz_t value);
ssa_value_t *ssa_value_char(const ssa_type_t *type, char value);
ssa_value_t *ssa_value_string(const ssa_type_t *type, text_view_t text);

ssa_value_t *ssa_value_from(map_t *types, const tree_t *expr);
ssa_value_t *ssa_value_noinit(const ssa_type_t *type);

ssa_value_t *ssa_value_literal(const ssa_type_t *type, ssa_literal_value_t value);
ssa_value_t *ssa_value_relative(const ssa_type_t *type, ssa_relative_value_t value);
ssa_value_t *ssa_value_opaque_literal(const ssa_type_t *type, mpz_t value);

///
/// operand api
///

ssa_operand_t operand_value(ssa_value_t *value);
