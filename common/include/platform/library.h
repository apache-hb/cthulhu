#pragma once

#include "platform/error.h"

typedef void *library_t;

void library_close(library_t library);

NODISCARD ALLOC(library_close)
library_t library_open(const char *path, cerror_t *error);

NODISCARD
void *library_get(library_t library, const char *symbol, cerror_t *error);
