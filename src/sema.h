#ifndef SEMA_H
#define SEMA_H

#include "ast.h"

typedef struct module_t module_t;

typedef struct {
    size_t length;
    size_t size;
    module_t *data;
} modules_t;

typedef struct module_t {
    const char *name;
    struct module_t *parent;
    modules_t *children;

    nodes_t *decls;
} module_t;

/* semantic errors */
extern int errors;

#define ERR(msg) { errors++; fprintf(stderr, "error" msg "\n"); }
#define ERRF(msg, ...) { errors++; fprintf(stderr, "error" msg "\n", __VA_ARGS__); }

#define REPORT(msg) ERR(": " msg)
#define REPORTF(msg, ...) ERRF(": " msg, __VA_ARGS__)

modules_t *compile_program(const char *start, FILE *source);

void dump_module(module_t *mod);

#endif /* SEMA_H */
