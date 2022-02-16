#include "cpp.h"

#include <ctype.h>

typedef struct {
    size_t depth; // depth into the file
    bool newline; // whether the last token was a newline

    bool once; // is #pragma once present
    size_t count; // how many times this file has been included
    
    file_t file; // the file ptr
} include_t;

// a file cache
typedef struct {
    reports_t *reports;
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

static char next_char(size_t *cursor, const char *data) {
    while (true) {
        char c = data[(*cursor)++];
        if (c == '\0') { return '\0'; }

        if (c != '\t' && c != '\r' && c != ' ') {
            return c;
        }
    }
}

static char *gather_util(const char *source, size_t *start, bool(*match)(char)) {
    size_t cursor = *start;
    while (true) {
        char c = source[cursor];

        if (match(c) || c == '\0') { break; }

        cursor += 1;
    }

    char *out = ctu_strndup(source + *start, cursor - *start);
    *start = cursor;
    return out;
}

static bool is_space(char c) {
    return isspace(c);
}

static bool match_normal_include(char c) {
    return c == '"';
}

static bool match_system_include(char c) {
    return c == '>';
}

static void do_include(cache_t *cache, const char *source, size_t *offset) {
    char start = next_char(offset, source);

    bool(*match)(char) = NULL;
    switch (start) {
    case '"': match = match_normal_include; break;
    case '<': match = match_system_include; break;
    default: {
        message_t *id = report(cache->reports, ERROR, NULL, "expected `\"` or `<`");
        report_note(id, "found `%c` instead", start);
        return;
    }
    }

    char *token = gather_util(source, offset, match);

    logverbose("include: %s", token);
}

static void do_preprocess(cache_t *cache, const char *source, size_t *offset) {
    char *token = gather_util(source, offset, is_space);

    if (streq(token, "include")) {
        do_include(cache, source, offset);
    } else if (streq(token, "pragma")) {
        do_pragma(cache, source, offset);
    } else {
        report(cache->reports, ERROR, NULL, "unknown preprocessor directive `%s`", token);
    }
}

static void process_file(cache_t *cache, scan_t *source) {
    size_t len = source->source.size;
    const char *data = source->source.text;

    bool fresh = true;

    size_t offset = 0;
    while (offset <= len) {
        char c = next_char(&offset, data);

        switch (c) {
        case '\n':
            fresh = true;
            break;

        case '\0':
            return;
        
        case '#':
            if (!fresh) {
                report(cache->reports, ERROR, NULL, "preprocessor directive not on a newline");
                break;
            }
            do_preprocess(cache, data, &offset);
            break;

        default:
            fresh = false;
            break;
        }
    }
}

scan_t *run_cpp(reports_t *reports, scan_t *root) {
    cache_t cache = { reports, map_new(MAP_MASSIVE) };
    process_file(&cache, root);
    return NULL;
}
