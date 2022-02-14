#include "cpp.h"

typedef struct {
    size_t depth; // depth into the file
    bool newline; // whether the last token was a newline

    bool once; // is #pragma once present
    size_t count; // how many times this file has been included
    
    file_t file; // the file ptr
} include_t;

// a file cache
typedef struct {
    map_t *files;
} cache_t;

static char *root_path(const char *root, const char *path) {
    char *base = ctu_basepath(root);
    char *out = ctu_pathjoin(base, path);
    return out;
}

static include_t *build_include(file_t file) {
    include_t *inc = ctu_malloc(sizeof(include_t));
    inc->depth = 0;
    inc->newline = true;
    inc->once = false;
    inc->count = 0;
    inc->file = file;
    return inc;
}

static include_t *new_include(reports_t *reports, node_t *node, const char *root, const char *path) {
    char *where = root_path(root, path);
    file_t file = ctu_fopen(where, "rb");
    if (file_valid(&file)) {
        return build_include(file);
    }

    report(reports, ERROR, node, "could not find include file `%s`", where);
    return NULL;
}

scan_t *run_cpp(reports_t *reports, scan_t *root) {
    cache_t cache = { map_new(MAP_MASSIVE) };
    return root;
}
