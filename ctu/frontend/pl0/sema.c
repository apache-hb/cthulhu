#include "sema.h"

#include <strings.h>
#include <ctype.h>

#include "ctu/util/report.h"

typedef struct {
    vector_t *globals;
} pl0_sema_t;

static pl0_sema_t *pl0_sema_new(void) {
    pl0_sema_t *sema = ctu_malloc(sizeof(pl0_sema_t));
    sema->globals = vector_new(4);
    return sema;
}

static node_t *pl0_get_global(pl0_sema_t *sema, const char *name) {
    for (size_t i = 0; i < vector_len(sema->globals); i++) {
        node_t *global = vector_get(sema->globals, i);
        if (strcasecmp(global->name, name) == 0) {
            return global;
        }
    }

    return NULL;
}

static char *pl0_sema_ident(pl0_node_t *node) {
    return node->ident;
}

#define PL0_SEMA_NUMBER(node) (node->number)

static void pl0_lower(char *str) {
    for (size_t i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

static void pl0_sema_const(pl0_sema_t *sema, pl0_node_t *node) {
    char *key = pl0_sema_ident(node->key);
    node_t *global = pl0_get_global(sema, key);

    if (global != NULL) {
        reportf(ERROR, node->scan, node->where, "const '%s' already defined", node->key);
    }

    pl0_lower(key);

    node_t *digit = ast_digit(node->scan, node->where, PL0_SEMA_NUMBER(node->value));
    node_t *value = ast_value(node->scan, node->where, false, key, digit);

    vector_push(&sema->globals, value);
}

static void pl0_sema_global(pl0_sema_t *sema, pl0_node_t *node) {
    char *key = pl0_sema_ident(node);
    node_t *global = pl0_get_global(sema, key);

    if (global != NULL) {
        reportf(ERROR, node->scan, node->where, "global '%s' already defined", node->key);
    }

    pl0_lower(key);

    node_t *digit = ast_digit_zero(node->scan, node->where);
    node_t *value = ast_value(node->scan, node->where, true, key, digit);

    vector_push(&sema->globals, value);
}

node_t *pl0_sema_program(pl0_sema_t *sema, pl0_node_t *node) {
    vector_t *consts = node->consts;
    vector_t *vars = node->vars;

    for (size_t i = 0; i < vector_len(consts); i++) {
        pl0_sema_const(sema, vector_get(consts, i));
    }

    for (size_t i = 0; i < vector_len(vars); i++) {
        pl0_sema_global(sema, vector_get(vars, i));
    }

    return ast_program(node->scan, node->where, sema->globals);
}

node_t *pl0_sema(pl0_node_t *program) {
    pl0_sema_t *sema = pl0_sema_new();
    return pl0_sema_program(sema, program);
}
