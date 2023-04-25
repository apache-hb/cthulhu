#pragma once

#include "scan/node.h"

#include <gmp.h>

#define CMDLTYPE where_t

typedef struct ap2_t ap2_t;

void ap2_on_string(ap2_t *self, const char *name, const char *value);
void ap2_on_bool(ap2_t *self, const char *name, bool value);
void ap2_on_int(ap2_t *self, const char *name, mpz_t value);
