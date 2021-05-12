#ifndef SCANNER_H
#define SCANNER_H

#include "ast.h"

typedef struct {
    void *scanner;
    const char *path;
    node_t *ast;
} scan_extra_t;

#endif /* SCANNER_H */
