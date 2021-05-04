#include "sema.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IS(node, type) if (node == NULL) { fprintf(stderr, "node was NULL\n"); return; } if (node->kind != type) { fprintf(stderr, "incorrect node type\n"); }

#define ERR(s, fmt) { add_error(s); fprintf(stderr, fmt); }
#define ERRF(s, fmt, ...) { add_error(s); fprintf(stderr, fmt, __VA_ARGS__); }

void
add_error(state_t *state)
{
    if (state->parent)
        add_error(state->parent);

    state->errors++;
}

state_t*
new_state(state_t *parent)
{
    state_t *out = malloc(sizeof(state_t));
    out->parent = parent;
    out->decls = empty_node_list();
    out->errors = 0;

    if (parent) {
        out->ret = parent->ret;
    }
    return out;
}

static int
name_equals(node_t *node, const char *name)
{
    if (node_is_decl(node)) {
        return strcmp(node->decl.name, name) == 0;
    }

    return 0;
}

static node_t* 
get_inttype(state_t *state) 
{
    if (state->parent)
        return get_inttype(state->parent);

    return state->inttype;
}

static node_t*
get_voidtype(state_t *state)
{
    if (state->parent)
        return get_voidtype(state->parent);

    return state->voidtype;
}

static int
types_equal(node_t *lhs, node_t *rhs)
{
    if (lhs == rhs) {
        return 1;
    }

    if (lhs == NULL || lhs == NULL) {
        return 0;
    } 

    if ((lhs->kind | rhs->kind) == NODE_BUILTIN_TYPE) {
        return strcmp(lhs->name, rhs->name) == 0;
    }

    return 0;
}

static node_t*
lookup_name(state_t *self, const char *name)
{
    for (size_t i = 0; i < self->decls->length; i++) {
        if (name_equals(self->decls->data + i, name)) {
            return self->decls->data + i;
        }
    }

    if (self->parent) {
        return lookup_name(self->parent, name);
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
sema_var(state_t *self, node_t *var, int add);

static node_t*
func_signature(state_t *self, node_t *func)
{
    if (func->kind != NODE_FUNC) {
        return NULL;
    }

    nodes_t *args = empty_node_list();
    for (size_t i = 0; i < func->decl.func.params->length; i++) {
        node_t *type = func->decl.func.params->data + i;
        args = node_append(args, lookup_type(self, type->decl.param.type));
    }
    node_t *res = func->decl.func.result;

    return new_closure(args, res);
}

static node_t*
resolve_type(state_t *self, node_t *expr)
{
    node_t *lhs, *rhs, *node;
    switch (expr->kind) {
    case NODE_UNARY: 
        return resolve_type(self, expr->unary.expr);
    case NODE_BINARY:
        lhs = resolve_type(self, expr->binary.lhs);
        rhs = resolve_type(self, expr->binary.rhs);
        if (!types_equal(lhs, rhs)) {
            ERR(self, "incompatible operands in binary expression");
            return NULL;
        }
        return lhs;
    case NODE_TERNARY:
        lhs = resolve_type(self, expr->ternary.yes);
        rhs = resolve_type(self, expr->ternary.no);
        if (!types_equal(lhs, rhs)) {
            ERR(self, "incompatible operands in ternary expression");
            return NULL;
        }
        return lhs;
    case NODE_DIGIT:
        return get_inttype(self);
    case NODE_NAME:
        node = lookup_name(self, expr->name);
        if (!node) {
            ERRF(self, "failed to find symbol named `%s`\n", expr->name);
            return NULL;
        }
        return resolve_type(self, node);
    case NODE_VAR:
        expr = sema_var(self, expr, 0);
        return expr->decl.var.type;
    case NODE_FUNC:
        return func_signature(self, expr);
    case NODE_PARAM:
        return lookup_type(self, expr->decl.param.type);
    default:
        ERRF(self, "unknown node type in resolve_type %d\n", expr->kind);
        return NULL;
    }
}

static int
node_is_callable(node_t *node)
{
    if (node->kind != NODE_CLOSURE) {
        return 0;
    }

    return 1;
}

static node_t*
sema_call(state_t *self, node_t *call)
{
    node_t *func = resolve_type(self, call->call.body);

    if (!node_is_callable(func)) {
        ERR(self, "calling non-closure expression");
        return call;
    }

    if (call->call.args->length != func->closure.args->length) {
        ERRF(self, 
            "calling function with wrong number of arguments, expected %zu got %zu\n",
            func->closure.args->length,
            call->call.args->length
        );
        return call;
    }

    for (size_t i = 0; i < call->call.args->length; i++) {
        node_t *param = lookup_type(self, func->closure.args->data + i);
        node_t *arg = resolve_type(self, call->call.args->data + i);

        if (!types_equal(param, arg)) {
            ERRF(self, "argument %zu is the wrong type\n", i);
        }
    }

    return call;
}

static node_t*
sema_stmt(state_t *self, node_t *stmt);

static node_t*
sema_compound(state_t *self, node_t *stmts)
{
    node_t *p = stmts->compound->data;
    
    for (size_t i = 0; i < stmts->compound->length; i++) {
        memcpy(p + i, sema_stmt(self, p + i), sizeof(node_t));
    }

    return stmts;
}

static node_t*
sema_return(state_t *self, node_t *stmt)
{
    if (stmt->expr == NULL && !types_equal(self->ret, get_voidtype(self))) {
        ERR(self, "void return in non-void function\n");
        return stmt;
    }

    node_t *res = resolve_type(self, stmt->expr);
    if (!types_equal(res, self->ret)) {
        ERR(self, "mismatched return type\n");
    }

    return stmt;
}

static node_t*
sema_stmt(state_t *self, node_t *stmt)
{
    state_t *nest;

    switch (stmt->kind) {
    case NODE_COMPOUND:
        nest = new_state(self);
        stmt = sema_compound(nest, stmt);
        break;
    case NODE_CALL:
        stmt = sema_call(self, stmt);
        break;
    case NODE_UNARY:
        ERR(self, "unary TODO\n");
        break;
    case NODE_BINARY:
        ERR(self, "binary TODO\n");
        break;
    case NODE_TERNARY:
        ERR(self, "ternary TODO\n");
        break;
    case NODE_VAR:
        stmt = sema_var(self, stmt, 1);
        break;
    case NODE_RETURN:
        stmt = sema_return(self, stmt);
        break;
    default:
        ERR(self, "unknown stmt kind\n");
        break;
    }

    return stmt;
}

static node_t* 
sema_func(state_t *self, node_t *func)
{
    state_t *nest = new_state(self);

    if (func->decl.func.result) {
        RESOLVE_TYPE(self, func->decl.func.result);
    } else {
        func->decl.func.result = get_voidtype(self);
    }

    for (size_t i = 0; i < func->decl.func.params->length; i++) {
        RESOLVE_TYPE(self, (func->decl.func.params->data + i)->decl.param.type);
        add_decl_unique(nest, (func->decl.func.params->data + i));
    }

    nest->ret = func->decl.func.result;
    func->decl.func.body = sema_stmt(nest, func->decl.func.body);

    return func;
}

static node_t*
sema_var(state_t *self, node_t *var, int add)
{
    node_t *init_type = resolve_type(self, var->decl.var.init);

    if (var->decl.var.type == NULL) {
        var->decl.var.type = init_type;
    } else {
        RESOLVE_TYPE(self, var->decl.var.type);
    }
    
    if (!types_equal(var->decl.var.type, init_type)) {
        ERRF(self, "var `%s` has incompatible type and initializer\n", var->decl.name);
    }

    if (self->parent && add) {
        add_decl_unique(self, var);
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
        return sema_var(self, decl, 1);
    default:
        ERR(self, "toplevel construct wasnt a decl");
        return NULL;
    }
}

void 
sema(state_t *self, node_t *program)
{
    self->parent = NULL;
    self->errors = 0;
    self->ret = NULL;

    IS(program, NODE_COMPOUND);

    for (size_t i = 0; i < program->compound->length; i++) {
        /* add all toplevel declarations first for order independant lookup */
        add_decl_unique(self, program->compound->data + i);
    }

    if (self->errors > 0) {
        return;
    }

    node_t *p = program->compound->data;

    for (size_t i = 0; i < program->compound->length; i++) {
        memcpy(p + i, sema_decl(self, p + i), sizeof(node_t));
    }
}
