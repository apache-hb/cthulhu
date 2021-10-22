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

static void trim_path(char *path) {
    size_t len = strlen(path);
    while (!endswith(path, PATH_SEP)) {
        path[len--] = '\0';
    }
    path[len] = '\0';
}

const char *find_include(const char *cwd, const char *path) {
    char *base = realpath(ctu_strdup(cwd), ctu_malloc(PATH_MAX + 1));
    trim_path(base);
    char *search = format("%s/%s", base, path);
    char *it = realpath(search, ctu_malloc(PATH_MAX + 1));
    if (access(it, F_OK) != -1) {
        return it;
    }
    
    size_t len = vector_len(includes);
    for (size_t i = 0; i < len; i++) {
        char *include = vector_get(includes, i);
        char *full = format("%s/%s", include, path);
        char *real = realpath(full, ctu_malloc(PATH_MAX + 1));
        if (access(real, F_OK) != -1) {
            return real;
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
