#pragma once

#include <stdbool.h>

typedef struct resultptr_t resultptr_t;
typedef struct resultptr_t *result_t;

result_t result_ok(int code);
result_t result_error(int code);

bool should_exit(result_t result);
int get_exit_code(result_t result);
