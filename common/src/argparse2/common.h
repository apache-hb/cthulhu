#pragma once

#include "scan/node.h"

#include <gmp.h>

#define APLTYPE where_t

typedef struct ap2_t ap2_t;
typedef struct ap2_param_t ap2_param_t;

void ap2_on_string(ap2_t *self, const ap2_param_t *param, const char *value);
void ap2_on_bool(ap2_t *self, const ap2_param_t *param, bool value);
void ap2_on_int(ap2_t *self, const ap2_param_t *param, mpz_t value);
void ap2_on_posarg(ap2_t *self, const char *value);

int ap2_get_opt(ap2_t *self, const char *name, ap2_param_t **param);
