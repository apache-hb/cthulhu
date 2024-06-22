#pragma once

#include <gmp.h>

#include "core/compiler.h"

CT_BEGIN_API

typedef struct logger_t logger_t;
typedef struct ref_ast_t ref_ast_t;

typedef enum eval_result_t {
    eEvalOk,
    eEvalTypeMismatch,
    eEvalOpaque,
    eEvalInvalid
} eval_result_t;

eval_result_t eval_expr(mpz_t result, logger_t *logs, ref_ast_t *expr);

CT_END_API
