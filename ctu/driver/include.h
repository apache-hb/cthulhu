#pragma once

#include "ctu/util/util.h"

void init_includes(vector_t *paths);
const char *find_include(const char *cwd, const char *path);
void set_cache(const char *path, void *data);
void *get_cache(const char *path);
vector_t *cached_data(void);
