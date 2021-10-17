#pragma once

#include "data.h"

type_t *compile_type(sema_t *sema, ctu_t *ctu);
type_t *common_digit(const type_t *lhs, const type_t *rhs);
type_t *next_digit(type_t *type);
type_t *common_type(const type_t *lhs, const type_t *rhs);
