#include "sema.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int errors = 0;

#define ERR(msg) { errors++; fprintf(stderr, "error: " msg "\n"); }
#define ERRF(msg, ...) { errors++; fprintf(stderr, "error: " msg "\n", __VA_ARGS__); }

state_t *new_state(state_t *parent)
{
    state_t *state = malloc(sizeof(state_t));
    state->parent = parent;
    state->decls = empty_list();
    return state;
}

static bool is_single_name_var(node_t *node)
{
    return node->decl.var.names->type == NODE_NAME;
}

static const char *get_single_var_name(node_t *node)
{
    return node->decl.var.names->text;
}

static const char *get_node_name(node_t *node)
{
    switch (node->type) {
    case NODE_VAR:
        if (is_single_name_var(node)) {
            return get_single_var_name(node);
        } else {
            ERR("destructuring var");
            return NULL;
        }
    case NODE_FUNC:
        return node->decl.name;
    default:
        ERRF("unnamed node %d\n", node->type);
        return NULL;
    }
}

static bool names_equal(node_t *lhs, node_t *rhs)
{
    const char *left = get_node_name(lhs);
    const char *right = get_node_name(rhs);

    return left != NULL && right != NULL 
        && strcmp(left, right) == 0;
}

static node_t *find_decl(state_t *state, node_t *decl)
{
    // check our current scope for this decl
    for (size_t i = 0; i < state->decls->length; i++) {
        node_t *node = state->decls->data + i;

        if (names_equal(node, decl)) {
            return node;
        }
    }

    // then go one up and check again if we can
    if (state->parent) {
        return find_decl(state->parent, decl);
    }

    return NULL;
}

static void add_decl(state_t *state, node_t *decl)
{
    switch (decl->type) {
    case NODE_FUNC:
    case NODE_VAR:
        break;
    
    default:
        ERRF("unsupported decl %d", decl->type);
        return;
    }

    if (find_decl(state, decl)) {
        ERRF("`%s` already defined", decl->decl.name);
        return;
    }

    // TODO: this can go when we have full support for all decl name variations
    if (get_node_name(decl) == NULL)
        return;

    list_add(state->decls, decl);
}

void nameresolve(state_t *state, node_t *node)
{
    for (size_t i = 0; i < node->scope.decls->length; i++) {
        node_t *decl = node->scope.decls->data + i;
        add_decl(state, decl);
    }

    for (size_t i = 0; i < state->decls->length; i++) {
        node_t *decl = state->decls->data + i;
        printf("decl %s\n", get_node_name(decl));
    }
}

void typecheck(state_t *state, node_t *node)
{
    (void)state;
    (void)node;
}

void constcheck(state_t *state, node_t *node)
{
    (void)state;
    (void)node;
}
