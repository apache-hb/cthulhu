#pragma once

#include "ctu/util/util.h"
#include "ctu/util/io.h"

void init_includes(vector_t *paths);
path_t *find_include(path_t *path);

void set_cache(const char *path, void *data);
void *get_cache(const char *path);
vector_t *cached_data(void);
