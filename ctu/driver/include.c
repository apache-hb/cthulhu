#include "include.h"

#include "ctu/util/report.h"
#include "ctu/util/str.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

static vector_t *includes = NULL;
static map_t *cache = NULL;

void init_includes(vector_t *paths) {
    includes = paths;
    cache = map_new(MAP_BIG);
}

path_t *find_include(path_t *path) {
    if (path_exists(path)) {
        return path;
    }

    size_t len = vector_len(includes);
    for (size_t i = 0; i < len; i++) {
        path_t *include = vector_get(includes, i);
        path_t *search = new_path(include->base, path_relative(path), path->ext);
        if (path_exists(search)) {
            return search;
        }
    }

    return NULL;
}

void set_cache(const char *path, void *data) {
    map_set(cache, path, data);
}

void *get_cache(const char *path) {
    return map_get(cache, path);
}

vector_t *cached_data(void) {
    return map_values(cache);
}
