#pragma once

#include "data.h"

void forward_type(sema_t *sema, const char *name, ctu_t *ctu);
void build_type(sema_t *sema, type_t *type);

type_t *compile_type(sema_t *sema, ctu_t *ctu);
type_t *common_digit(const type_t *lhs, const type_t *rhs);
type_t *next_digit(type_t *type);
type_t *common_type(const type_t *lhs, const type_t *rhs);

lir_t *implicit_convert_expr(sema_t *sema, lir_t *expr, const type_t *type);
lir_t *explicit_convert_expr(sema_t *sema, lir_t *expr, const type_t *type);

char *ctu_type_format(const type_t *type);
