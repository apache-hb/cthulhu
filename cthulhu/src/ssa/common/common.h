#pragma once

#include "cthulhu/ssa/ssa.h"

///
/// type api
///

ssa_type_t *ssa_type_new(ssa_kind_t kind, const char *name);

ssa_type_t *ssa_type_empty(const char *name);
ssa_type_t *ssa_type_unit(const char *name);
ssa_type_t *ssa_type_bool(const char *name);
ssa_type_t *ssa_type_digit(const char *name, sign_t sign, digit_t digit);
ssa_type_t *ssa_type_string(const char *name); // TODO: encoding
ssa_type_t *ssa_type_qualify(const char *name, quals_t quals, ssa_type_t *type);
ssa_type_t *ssa_type_closure(const char *name, ssa_type_t *result, typevec_t *params, bool variadic);

ssa_type_t *ssa_type_from(const h2_t *type);

ssa_type_t *ssa_type_common(const ssa_type_t *lhs, const ssa_type_t *rhs);

///
/// value api
///

ssa_value_t *ssa_value_new(const ssa_type_t *type);

ssa_value_t *ssa_value_empty(const ssa_type_t *type);
ssa_value_t *ssa_value_unit(const ssa_type_t *type);
ssa_value_t *ssa_value_bool(const ssa_type_t *type, bool value);
ssa_value_t *ssa_value_digit(const ssa_type_t *type, const mpz_t value);
ssa_value_t *ssa_value_string(const ssa_type_t *type, const char *value, size_t length);

ssa_value_t *ssa_value_from(const h2_t *expr);
