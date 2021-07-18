#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const char *path;
    void *ast;

    const char *text;
    size_t offset;
    size_t size;
} scanner_t;

typedef int64_t loc_t;

typedef struct {
    loc_t first_line;
    loc_t first_column;
    loc_t last_line;
    loc_t last_column;
} where_t;

where_t merge_locations(where_t begin, where_t end);

void flex_init(where_t *where, int line);
int flex_get(scanner_t *scanner, char *out, int size);
void flex_update(where_t *where, const char *text);
void free_scanner(scanner_t *scanner);

#define YYLTYPE where_t
