#include "sema.h"

#include <stdio.h>
#include <string.h>

#define IS(node, type) if (node == NULL) { fprintf(stderr, "node was NULL\n"); return; } if (node->kind != type) { fprintf(stderr, "incorrect node type\n"); }

#define ERR(s, fmt) { s->errors++; fprintf(stderr, fmt); }
#define ERRF(s, fmt, ...) { s->errors++; fprintf(stderr, fmt, __VA_ARGS__); }

static int
name_equals(node_t *node, char *name)
{
    if (node_is_decl(node)) {
        return strcmp(node->decl.name, name) == 0;
    }
    return 0;
}

static node_t*
lookup_name(state_t *self, char *name)
{
    for (size_t i = 0; i < self->decls->length; i++) {
        if (name_equals(self->decls->data + i, name)) {
            return self->decls->data + i;
        }
    }

    return NULL;
}

static void
add_decl_unique(state_t *self, node_t *node)
{
    if (lookup_name(self, node->decl.name)) {
        ERRF(self, "decl `%s` already defined\n", node->decl.name);
        return;
    }

    self->decls = node_append(self->decls, node);
}

#define RESOLVE_TYPE(state, it) it = lookup_type(state, it)

static node_t*
lookup_type(state_t *self, node_t *unresolved)
{
    if (unresolved->kind == NODE_POINTER) {
        RESOLVE_TYPE(self, unresolved->type);
        return unresolved;
    }

    node_t *node = lookup_name(self, unresolved->decl.name);
    if (!node) {
        ERRF(self, "failed to resolve type `%s`\n", unresolved->decl.name);
        return NULL;
    }

    if (node_is_type(node)) {
        return node;
    }

    ERRF(self, "%s is not a type\n", node->decl.name);
    return NULL;
}

static node_t*
resolve_type(state_t *self, node_t *expr)
{
    node_t *lhs, *rhs;
    switch (expr->kind) {
    case NODE_UNARY: 
        return resolve_type(self, expr->unary.expr);
    case NODE_BINARY:
        lhs = resolve_type(self, expr->binary.lhs);
        rhs = resolve_type(self, expr->binary.rhs);
        if (lhs != rhs) {
            ERR(self, "incompatible operands in binary expression");
            return NULL;
        }
        return lhs;
    case NODE_TERNARY:
        lhs = resolve_type(self, expr->ternary.yes);
        rhs = resolve_type(self, expr->ternary.no);
        if (lhs != rhs) {
            ERR(self, "incompatible operands in ternary expression");
            return NULL;
        }
        return lhs;
    case NODE_DIGIT:
        return self->inttype;
    default:
        ERR(self, "unknown node type in resolve_type");
        return NULL;
    }
}

static node_t* 
sema_func(state_t *self, node_t *func)
{
    if (func->decl.func.result) {
        RESOLVE_TYPE(self, func->decl.func.result);
    } else {
        func->decl.func.result = self->voidtype;
    }

    for (size_t i = 0; i < func->decl.func.params->length; i++) {
        RESOLVE_TYPE(self, (func->decl.func.params->data + i)->decl.param.type);
    }

    return func;
}

static node_t*
sema_var(state_t *self, node_t *var)
{
    node_t *init = resolve_type(self, var->decl.var.init);
    
    printf("var %s %s\n", var->decl.name, init->decl.name);

    if (init->decl.var.type == NULL) {
        printf("inferring\n");
        init->decl.var.type = init;
    } else {
        printf("resolving %p\n", init->decl.var.type);
        RESOLVE_TYPE(self, init->decl.var.type);
    }

    printf("var %s\n", var->decl.name);
    dump_node(init->decl.var.type);
    printf("\n");
    
    if (init->decl.var.type != init) {
        ERRF(self, "var `%s` has incompatible type and initializer\n", var->decl.name);
    }

    return var;
}

static node_t*
sema_decl(state_t *self, node_t *decl)
{
    switch (decl->kind) {
    case NODE_FUNC: 
        return sema_func(self, decl);
    case NODE_VAR:
        return sema_var(self, decl);
    default:
        ERR(self, "toplevel construct wasnt a decl");
        return NULL;
    }
}

void 
sema(state_t *self, node_t *program)
{
    self->errors = 0;

    IS(program, NODE_COMPOUND);

    for (size_t i = 0; i < program->compound->length; i++) {
        /* add all toplevel declarations first for order independant lookup */
        add_decl_unique(self, program->compound->data + i);
    }

    node_t *p = program->compound->data;

    for (size_t i = 0; i < program->compound->length; i++) {
        memcpy(p + i, sema_decl(self, p + i), sizeof(node_t));
    }
}
