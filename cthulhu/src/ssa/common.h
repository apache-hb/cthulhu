#pragma once

#include "cthulhu/ssa/ssa.h"

ssa_type_t *ssa_type_new(ssa_kind_t kind, const char *name);

ssa_type_t *ssa_type_empty(const char *name);
ssa_type_t *ssa_type_unit(const char *name);
ssa_type_t *ssa_type_bool(const char *name);
ssa_type_t *ssa_type_digit(const char *name, sign_t sign, digit_t digit);
ssa_type_t *ssa_type_qualify(const char *name, quals_t quals, ssa_type_t *type);
ssa_type_t *ssa_type_closure(const char *name, ssa_type_t *result, typevec_t *params);

ssa_type_t *ssa_type_from(const h2_t *type);
