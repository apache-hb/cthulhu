#include "sema.h"

#include "ctu/util/report.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static sema_t *new_sema(sema_t *parent) {
    sema_t *sema = malloc(sizeof(sema_t));
    sema->parent = parent;
    sema->decls = ast_list(NULL);
    return sema;
}

static node_t *find_decl(sema_t *sema, const char *name) {
    for (size_t i = 0; i < sema->decls->len; i++) {
        node_t *decl = sema->decls->data + i;
        if (strcmp(decl->name, name) == 0) {
            return decl;
        }
    }

    return !!sema->parent
        ? find_decl(sema->parent, name)
        : NULL;
}

static void add_decl(sema_t *sema, node_t *decl) {
    if (find_decl(sema, decl->name)) {
        reportf(LEVEL_ERROR, decl, "decl %s already defined", decl->name);
    }

    ast_append(sema->decls, decl);
}

sema_t *resolve(nodes_t *nodes) {
    sema_t *sema = new_sema(NULL);
    sema->nodes = nodes;

    for (size_t i = 0; i < nodes->len; i++) {
        add_decl(sema, nodes->data + i);
    }

    return sema;
}

void typecheck(sema_t *sema) {
    (void)sema;
}
