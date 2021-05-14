#ifndef SCANNER_H
#define SCANNER_H

#include "ast.h"

typedef struct scan_extra_t {
    const char *path;
    path_t *mod;
    node_t *ast;
} scan_extra_t;

#endif /* SCANNER_H */
