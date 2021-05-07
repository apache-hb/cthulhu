#include "sema.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERR(s, fmt) { add_error(s); fprintf(stderr, fmt); }
#define ERRF(s, fmt, ...) { add_error(s); fprintf(stderr, fmt, __VA_ARGS__); }

static state_t*
root_state(state_t *state) { return state->parent ? root_state(state->parent) : state; }

static void
new_state(state_t *out, state_t *parent)
{
    out->parent = parent;
    out->decls = empty_node_list();
    out->ret = parent->ret;
    out->errors = 0;
}

static void 
add_error(state_t *state) { root_state(state)->errors++; }

static const char*
get_decl_name(node_t *decl) { return decl->decl.name; }

static node_t*
get_inttype(state_t *state) { return root_state(state)->inttype; }

static node_t*
get_booltype(state_t *state) { return root_state(state)->booltype; }

static node_t*
get_voidtype(state_t *state) { return root_state(state)->voidtype; }

static node_t*
lookup_name(state_t *self, const char *name)
{
    for (size_t i = 0; i < self->decls->length; i++) {
        node_t *decl = self->decls->data + i;
        const char *id = get_decl_name(decl);

        if (strcmp(id, name) == 0) {
            return decl;
        }
    }

    if (self->parent) {
        return lookup_name(self->parent, name);
    }

    return NULL;
}

static node_t*
add_decl(state_t *self, node_t *decl)
{
    if (lookup_name(self, decl->decl.name)) {
        ERRF(self, "multiple definitions of `%s`\n", decl->decl.name);
    } else {
        self->decls = node_append(self->decls, decl);
    }

    return decl;
}

static int
convertible_to(state_t *self, node_t *lhs, node_t *rhs) {
    if (lhs->kind == NODE_BUILTIN_TYPE && rhs->kind == NODE_BUILTIN_TYPE) {
        return strcmp(lhs->decl.name, rhs->decl.name) == 0;
    }

    if (lhs->kind == NODE_POINTER && rhs->kind == NODE_POINTER) {
        return convertible_to(self, lhs->type, rhs->type);
    }

    return 0;
}

static node_t*
sema_stmt(state_t *self, node_t *decl);

static node_t*
resolve_type(state_t *self, node_t *node);

static node_t*
sema_var(state_t *self, node_t *decl)
{
    node_t *type = NULL;

    if (!decl->decl.var.init && !decl->decl.var.type) {
        ERRF(self, "`%s` either a type or assignment is required\n", decl->decl.name);
        return decl;
    }

    /* get the type of init */
    if (decl->decl.var.init) {
        type = resolve_type(self, decl->decl.var.init);
    }

    if (!decl->decl.var.type) {
        decl->decl.var.type = type;
    } else {
        decl->decl.var.type = resolve_type(self, decl->decl.var.type);

        if (decl->decl.var.init) {
            if (!convertible_to(self, decl->decl.var.type, type)) {
                ERRF(self, "`%s` cannot assign unrelated types\n", decl->decl.name);
            }
        }
    }

    if (decl->mut) {
        decl->decl.var.type->mut = 1;
    }

    if (convertible_to(self, get_voidtype(self), decl->decl.var.type)) {
        ERRF(self, "cannot initialize `%s` with void\n", decl->decl.name);
    }

    return decl;
}

static node_t*
resolve_call(state_t *self, node_t *node)
{
    node_t *func = resolve_type(self, node->call.body);

    if (func->kind != NODE_CLOSURE) {
        ERR(self, "cannot call non-closure type\n");
    }

    if (func->closure.args->length != node->call.args->length) {
        ERRF(self, 
            "calling function with wrong number of parameters, expected `%zu` got `%zu`\n",
            func->closure.args->length,
            node->call.args->length
        );
    }

    for (size_t i = 0; i < func->closure.args->length; i++) {
        node_t *arg = resolve_type(self, func->closure.args->data + i);
        node_t *it = resolve_type(self, node->call.args->data + i);

        if (!convertible_to(self, arg, it)) {
            ERRF(self, "argument `%zu` has an incompatible type\n", i);
        }
    }

    return func->closure.result;
}

static node_t*
sema_return(state_t *self, node_t *decl)
{
    node_t *result = decl->expr
        ? resolve_type(self, decl->expr)
        : get_voidtype(self);

    if (!convertible_to(self, self->ret, result)) {
        ERR(self, "incompatible return types\n");
    }

    return decl;
}

static node_t*
sema_branch(state_t *self, node_t *decl)
{
    if (decl->branch.cond) {
        node_t *type = resolve_type(self, decl->branch.cond);
        if (!convertible_to(self, get_booltype(self), type) && !convertible_to(self, get_inttype(self), type)) {
            ERR(self, "conditional must evaluate to a bool or an int");
        }
    }

    decl->branch.body = sema_stmt(self, decl->branch.body);

    if (decl->branch.next) {
        decl->branch.next = sema_branch(self, decl->branch.next);
    }

    return decl;
}

static node_t*
sema_assign(state_t *self, node_t *decl)
{
    node_t *rhs = resolve_type(self, decl->assign.expr),
           *lhs = resolve_type(self, decl->assign.old);

    if (!convertible_to(self, lhs, rhs)) {
        ERR(self, "assigning incompatible types\n");
    }

    if (!lhs->mut) {
        ERR(self, "cannot assign to an immutable value\n");
    }

    return decl;
}

static node_t*
sema_stmt(state_t *self, node_t *decl)
{
    state_t nest;
    size_t i;

    switch (decl->kind) {
    case NODE_COMPOUND:
        new_state(&nest, self);
        for (i = 0; i < decl->compound->length; i++) {
            decl->compound = node_replace(decl->compound, i, sema_stmt(&nest, decl->compound->data + i));
        }
        break;
    case NODE_CALL:
        resolve_call(self, decl);
        break;
    case NODE_VAR:
        decl = add_decl(self, sema_var(self, decl));
        break;
    case NODE_DIGIT:
        break;
    case NODE_BINARY:
        resolve_type(self, decl);
        break;
    case NODE_RETURN:
        decl = sema_return(self, decl);
        break;
    case NODE_BRANCH:
        decl = sema_branch(self, decl);
        break;
    case NODE_ASSIGN:
        decl = sema_assign(self, decl);
        break;
    default:
        ERRF(self, "sema_stmt(%d) unknown kind\n", decl->kind);
        break;
    }

    return decl;
}

static node_t*
sema_func(state_t *self, node_t *decl)
{
    state_t nest;
    node_t *result = decl->decl.func.result 
        ? resolve_type(self, decl->decl.func.result) 
        : get_voidtype(self);

    decl->decl.func.result = result;
    new_state(&nest, self);
    nest.ret = result;

    for (size_t i = 0; i < decl->decl.func.params->length; i++) {
        node_t *param = decl->decl.func.params->data + i;
        param->decl.param = resolve_type(self, param->decl.param);
        add_decl(&nest, param);
    }

    if (decl->decl.func.body) {
        decl->decl.func.body = sema_stmt(&nest, decl->decl.func.body);
    }

    return decl;
}

static node_t*
sema_record(state_t *self, node_t *decl)
{
    for (size_t i = 0; i < decl->decl.fields->length; i++) {
        node_t *field = decl->decl.fields->data + i;
        field->decl.param = resolve_type(self, field->decl.param);
        node_replace(decl->decl.fields, i, field);
    }

    return decl;
}

static node_t*
sema_decl(state_t *self, node_t *decl)
{
    switch (decl->kind) {
    case NODE_FUNC: 
        return sema_func(self, decl);
    case NODE_VAR:
        return sema_var(self, decl);
    case NODE_RECORD:
        return sema_record(self, decl);
    default:
        ERRF(self, "sema_decl unknown kind %d\n", decl->kind);
        return decl;
    }
}

static node_t*
resolve_unary(state_t *self, node_t *node)
{
    node_t *expr = node->unary.expr;
    node_t *type = resolve_type(self, expr);

    switch (node->unary.op) {
    case UNARY_NOT:
        if (!convertible_to(self, get_booltype(self), type)) {
            ERR(self, "unary not applied to non-boolean type\n");
        }
        break;
    case UNARY_ABS: case UNARY_NEG:
        if (!convertible_to(self, get_inttype(self), type)) {
            ERR(self, "unary abs or neg applied to non-integer type\n");
        }
        break;
    case UNARY_REF:
        if (expr->kind != NODE_NAME) {
            ERR(self, "cannot take reference to a non-rvalue\n");
        }
        type = new_pointer(type);
        break;
    case UNARY_DEREF:
        if (type->kind != NODE_POINTER) {
            ERR(self, "cannot derefence a non pointer\n");
        } else {
            type = type->type;
        }
        break;
    default:
        break;
    }

    return type;
}

static node_t*
resolve_binary(state_t *self, node_t *node)
{
    node_t *lhs = resolve_type(self, node->binary.lhs),
           *rhs = resolve_type(self, node->binary.rhs);

    if (!convertible_to(self, lhs, rhs)) {
        ERR(self, "incompatible binary operands\n");
    }

    switch (node->binary.op) {
    case BINARY_ADD: case BINARY_SUB:
    case BINARY_DIV: case BINARY_MUL: case BINARY_REM:
        if (!convertible_to(self, get_inttype(self), lhs)) {
            ERR(self, "math operator applied to non-int operands\n");
        }
        break;
    }

    return lhs;
}

static node_t*
resolve_func(state_t *self, node_t *node)
{
    node_t *res = node->decl.func.result
        ? resolve_type(self, node->decl.func.result)
        : get_voidtype(self);
    nodes_t *args = empty_node_list();

    for (size_t i = 0; i < node->decl.func.params->length; i++) {
        args = node_append(args, resolve_type(self, 
            (node->decl.func.params->data + i)->decl.param
        ));
    }

    return new_closure(args, res);
}

static node_t*
resolve_access(state_t *self, node_t *node)
{
    const char *field = node->access.field;
    node_t *record = resolve_type(self, node->access.expr);

    if (record->kind != NODE_RECORD) {
        ERR(self, "left hand side of access must be a record type\n");
        return node;
    }

    for (size_t i = 0; i < record->decl.fields->length; i++) {
        node_t *decl = record->decl.fields->data + i;
        if (strcmp(decl->decl.name, field) == 0) {
            return resolve_type(self, decl->decl.param);
        }
    }

    ERRF(self, "record `%s` missing field `%s`\n", record->decl.name, field);
    return get_voidtype(self);
}

static node_t*
resolve_type(state_t *self, node_t *node)
{
    switch (node->kind) {
    case NODE_DIGIT: 
        return get_inttype(self);
    case NODE_BOOL: 
        return get_booltype(self);
    case NODE_BUILTIN_TYPE: 
        return node;
    case NODE_NAME: case NODE_TYPENAME:
        return resolve_type(self, lookup_name(self, node->name));
    case NODE_UNARY:
        return resolve_unary(self, node);
    case NODE_BINARY:
        return resolve_binary(self, node);
    case NODE_POINTER:
        node->type = resolve_type(self, node->type);
        return node;
    case NODE_CALL:
        return resolve_call(self, node);
    case NODE_VAR:
        return resolve_type(self, sema_var(self, node)->decl.var.type);
    case NODE_FUNC:
        return resolve_func(self, node);
    case NODE_PARAM:
        return resolve_type(self, node->decl.param);
    case NODE_RECORD:
        return node;
    case NODE_ACCESS:
        return resolve_access(self, node);
    default:
        ERRF(self, "resolve_type(%d) unknown kind\n", node->kind);
        return node;
    }
}

void 
sema(state_t *self, node_t *program)
{
    self->parent = NULL;
    self->errors = 0;

    for (size_t i = 0; i < program->compound->length; i++) {
        /* add all toplevel declarations first for order independant lookup */
        add_decl(self, program->compound->data + i);
    }

    if (self->errors > 0) {
        return;
    }

    node_t *p = program->compound->data;

    for (size_t i = 0; i < program->compound->length; i++) {
        node_replace(program->compound, i, sema_decl(self, p + i));
    }
}
