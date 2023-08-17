#pragma once

typedef struct h2_t h2_t;

void *util_select_decl(h2_t *sema, const size_t *tags, size_t len, const char *name);
