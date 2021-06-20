#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *path;
    void *ast;

    void *handle;
    int(*next)(void*);

    char *text;
    size_t len, size;
} scanner_t;

typedef int64_t loc_t;

typedef struct {
    loc_t first_line;
    loc_t first_column;
    loc_t last_line;
    loc_t last_column;
} where_t;

void flex_init(where_t *where, int line);
int flex_get(scanner_t *scanner, char *out, int size);
void flex_update(where_t *where, int lineno, int length, const char *text);

#define YYLTYPE where_t
