#pragma once

#include <stddef.h>

typedef struct {
    const char *path;
    struct node_t *node;

    void *file;
    int(*next)(void*);

    char *text;
    size_t len, size;
} scanner_t;

typedef struct {
    int first_line;
    int first_column;
    int last_line;
    int last_column;
} where_t;

void flex_init(where_t *where);
int flex_get(scanner_t *scanner, char *out);
void flex_update(where_t *where, int lineno, int length, const char *text);

#define CTU_LTYPE where_t
#define YY_USER_INIT flex_init(yylloc);
