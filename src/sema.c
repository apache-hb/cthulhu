#include "sema.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERR(s, fmt) { add_error(s); fprintf(stderr, fmt); }
#define ERRF(s, fmt, ...) { add_error(s); fprintf(stderr, fmt, __VA_ARGS__); }

static state_t*
root_state(state_t *state) { return state->parent ? root_state(state->parent) : state; }

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

static void
add_decl(state_t *self, node_t *decl)
{
    const char *name = decl->decl.name;

    if (lookup_name(self, name)) {
        ERRF(self, "`%s` has already been defined\n", name);
    } else {
        self->decls = node_append(self->decls, decl);
    }
}

static int
types_equal(node_t *lhs, node_t *rhs)
{
    if (lhs == rhs) {
        return 1;
    }

    if (lhs->kind == NODE_BUILTIN_TYPE && rhs->kind == NODE_BUILTIN_TYPE) {
        return strcmp(lhs->builtin.name, rhs->builtin.name) == 0;
    }

    if (lhs->kind == NODE_POINTER && rhs->kind == NODE_POINTER) {
        return types_equal(lhs->type, rhs->type);
    }

    return 0;
}

state_t
new_state(state_t *parent)
{
    state_t out;
    out.parent = parent;
    out.decls = empty_node_list();
    out.errors = 0;

    if (parent) {
        out.ret = parent->ret;
    } else {
        out.ret = NULL;
    }

    return out;
}

#if 0
#define IS(node, type) if (node == NULL) { fprintf(stderr, "node was NULL\n"); return; } if (node->kind != type) { fprintf(stderr, "incorrect node type\n"); }

static const char*
node_get_name(node_t *node)
{
    switch (node->kind) {
    case NODE_TYPENAME: case NODE_NAME:
    case NODE_PARAM: case NODE_FUNC: case NODE_VAR:
        return node->name;
    case NODE_BUILTIN_TYPE:
        return node->builtin.name;
    default:
        printf("node_get_name %d\n", node->kind);
        return NULL;
    }
}

static int
name_equals(node_t *node, const char *name)
{
    if (node_is_decl(node)) {
        return strcmp(node_get_name(node), name) == 0;
    }

    return 0;
}

static node_t*
get_chartype(state_t *state)
{
    if (state->parent)
        return get_chartype(state->parent);

    return state->chartype;
}

static node_t*
get_stringtype(state_t *state)
{
    return new_pointer(get_chartype(state));
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
        return strcmp(lhs->builtin.name, rhs->builtin.name) == 0;
    }

    if (lhs->kind == rhs->kind) {
        return types_equal(lhs->type, rhs->type) && lhs->mut == rhs->mut;
    }

    return 0;
}

static node_t*
lookup_name(state_t *self, const char *name)
{
    if (self->parent) {
        self->looked = name;
    }

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
    switch (unresolved->kind) {
    case NODE_POINTER:
        RESOLVE_TYPE(self, unresolved->type);
        return unresolved;
    default:
        break;
    }

    const char *name = node_get_name(unresolved);
    node_t *node = lookup_name(self, name);
    if (!node) {
        ERRF(self, "failed to resolve type `%s`\n", name);
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

static int
binary_op_compatible(state_t *self, node_t *type, binary_op_t op) {
    (void)op;
    if (types_equal(type, get_inttype(self))) {
        return 1;
    }

    return 0;
}

static int
unary_op_compatible(state_t *self, node_t *type, binary_op_t op) {
    (void)op;
    if (types_equal(type, get_inttype(self))) {
        return 1;
    }

    return 0;
}

static node_t*
resolve_type(state_t *self, node_t *expr)
{
    node_t *lhs, *rhs, *node;
    switch (expr->kind) {
    case NODE_TYPENAME:
        node = lookup_type(self, expr);
        return resolve_type(self, node);
    case NODE_BUILTIN_TYPE:
        return expr;
    case NODE_STRING:
        return get_stringtype(self);
    case NODE_UNARY: 
        node = resolve_type(self, expr->unary.expr);
        if (expr->unary.op == UNARY_DEREF && node->kind == NODE_POINTER) {
            return resolve_type(self, node->type);
        }
        if (!unary_op_compatible(self, node, expr->unary.op)) {
            ERR(self, "type not valid for unary operation\n");
        }
        return node;
    case NODE_BINARY:
        lhs = resolve_type(self, expr->binary.lhs);
        rhs = resolve_type(self, expr->binary.rhs);
        if (!types_equal(lhs, rhs)) {
            ERR(self, "incompatible operands in binary expression\n");
        }
        if (!binary_op_compatible(self, rhs, expr->binary.op)) {
            ERR(self, "types not valid for binary operation\n");
        }
        return lhs;
    case NODE_TERNARY:
        lhs = resolve_type(self, expr->ternary.yes);
        rhs = resolve_type(self, expr->ternary.no);
        if (!types_equal(lhs, rhs)) {
            ERR(self, "incompatible operands in ternary expression\n");
        }
        return lhs;
    case NODE_CALL:
        node = resolve_type(self, expr->call.body);
        return resolve_type(self, node->closure.result);
    case NODE_DIGIT:
        return get_inttype(self);
    case NODE_BOOL:
        return get_booltype(self);
    case NODE_NAME:
        node = lookup_name(self, expr->name);
        if (!node) {
            ERRF(self, "failed to find symbol named `%s`\n", expr->name);
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
        ERRF(self, "void return in non-void function `%s`\n", self->name);
        return stmt;
    }

    node_t *res = resolve_type(self, stmt->expr);
    if (!types_equal(res, self->ret)) {
        ERRF(self, "mismatched return type in function `%s`\n", self->name);
    }

    return stmt;
}

static int
valid_in_assign_lhs(state_t *self, node_t *stmt)
{
    node_t *res;
    
    switch (stmt->kind) {
    case NODE_NAME:
        res = lookup_name(self, stmt->name);
        break;
    default:
        res = resolve_type(self, stmt);
        break;
    }


    printf("kind %d\n", res->kind);
    switch (res->kind) {
    case NODE_POINTER:
        return res->type->mut;
    case NODE_BUILTIN_TYPE:
        return 1;
    default:
        return res->mut;
    }
}

static node_t*
sema_assign(state_t *self, node_t *stmt)
{
    int valid = valid_in_assign_lhs(self, stmt->assign.old);

    if (!valid) {
        ERRF(self, "cannot modify a non-mvalue `%s` in `%s`\n", self->looked, self->name);
        return stmt;
    }
    
    node_t *res = resolve_type(self, stmt->assign.old);
    node_t *other = resolve_type(self, stmt->assign.expr);

    if (!types_equal(res, other)) {
        ERRF(self, "cannot assign incompatible types in `%s`\n", self->name);
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
    case NODE_ASSIGN:
        stmt = sema_assign(self, stmt);
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
    nest->name = func->decl.name;
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
    
    if (types_equal(var->decl.var.type, get_voidtype(self))) {
        ERRF(self, "var `%s` has a type of void\n", var->decl.name);
    }

    if (!types_equal(var->decl.var.type, init_type)) {
        ERRF(self, "var `%s` has incompatible type and initializer\n", var->decl.name);
    }

    if (!var->mut) {
        var->decl.var.type->mut = 0;
    }

    if (self->parent && add) {
        add_decl_unique(self, var);
    }

    return var;
}

#endif

static node_t*
typeof_expr(state_t *self, node_t *expr);

static int
can_reference(state_t *self, node_t *expr)
{
    switch (expr->kind) {
    case NODE_NAME: 
        return 1;
    case NODE_TERNARY:
        return can_reference(self, expr->ternary.yes) 
            && can_reference(self, expr->ternary.no);
    default:
        return 0;
    }
}

static node_t*
typeof_unary(state_t *self, node_t *unary)
{
    unary_op_t op = unary->unary.op;
    node_t *expr = unary->unary.expr;
    node_t *type = typeof_expr(self, unary->unary.expr);

    if (op == UNARY_REF) {
        if (!can_reference(self, expr)) {
            ERR(self, "cannot take reference to an immediate\n");
        }
        return new_pointer(type);
    }

    if (op == UNARY_DEREF) {
        if (type->kind != NODE_POINTER) {
            ERR(self, "cannot derefence non-pointer type\n");
        }
    }

    return type;
}

static node_t*
sema_var(state_t *self, node_t *decl);

static node_t*
flatten_type(state_t *self, node_t *type);

static node_t*
typeof_decl(state_t *self, node_t *decl)
{
    node_t *node;
    switch (decl->kind) {
    case NODE_VAR:
        node = sema_var(self, decl);
        return node->decl.var.type;
    case NODE_PARAM:
        node = flatten_type(self, decl->decl.param);
        decl->decl.param = node;
        return decl->decl.param;
    default:
        ERRF(self, "typeof_decl unknown kind %d\n", decl->kind);
        return NULL;
    }
}

static node_t*
typeof_expr(state_t *self, node_t *expr)
{
    node_t *lhs, *rhs, *node;
    switch (expr->kind) {
    case NODE_DIGIT: 
        return get_inttype(self);
    case NODE_BOOL:
        return get_booltype(self);
    case NODE_UNARY:
        return typeof_unary(self, expr);
    case NODE_BINARY:
        lhs = typeof_expr(self, expr->binary.lhs);
        rhs = typeof_expr(self, expr->binary.rhs);
        
        if (!types_equal(lhs, rhs)) {
            ERR(self, "incompatible binary operands\n");
        }

        /* TODO: check binary operands are compatible */

        return lhs;
    case NODE_NAME:
        node = lookup_name(self, expr->name);
        return typeof_decl(self, node);
    case NODE_TERNARY:
        lhs = typeof_expr(self, expr->ternary.yes);
        rhs = typeof_expr(self, expr->ternary.no);

        if (!types_equal(lhs, rhs)) {
            ERR(self, "incompatible ternary branch types\n");
        }

        return lhs;
    default:
        ERRF(self, "typeof_expr unknown kind %d\n", expr->kind);
        return NULL;
    }
}

static node_t*
flatten_type(state_t *self, node_t *type)
{
    node_t *node;
    switch (type->kind) {
    case NODE_TYPENAME:
        node = lookup_name(self, type->name);
        return flatten_type(self, node);
    case NODE_BUILTIN_TYPE:
        return type;
    case NODE_POINTER:
        type->type = flatten_type(self, type->type);
        return type;
    default:
        ERRF(self, "flatten_type unknown kind %d\n", type->kind);
        return NULL;
    }
}

static node_t*
sema_return(state_t *self, node_t *stmt)
{
    node_t *type = stmt->expr ? typeof_expr(self, stmt->expr) : get_voidtype(self);

    if (!types_equal(self->ret, type)) {
        ERR(self, "incompatible return type\n");
    }

    return stmt;
}

#define DECL_TYPE decl->decl.var.type

static node_t*
sema_var(state_t *self, node_t *decl)
{
    node_t *init = decl->decl.var.init;

    node_t *type = init ? typeof_expr(self, init) : NULL;

    if (init && !DECL_TYPE) {
        DECL_TYPE = type;
    }
    
    DECL_TYPE = flatten_type(self, DECL_TYPE);
    
    if (type && !types_equal(DECL_TYPE, type)) {
        ERRF(self, "incompatible types when initializing `%s`\n", decl->decl.name);
    }

    return decl;
}

#undef DECL_TYPE

static node_t*
sema_assign(state_t *self, node_t *stmt)
{
    node_t *lhs = typeof_expr(self, stmt->assign.old);
    node_t *rhs = typeof_expr(self, stmt->assign.expr);

    if (!types_equal(lhs, rhs)) {
        ERR(self, "cannot assign unrelated types\n");
    }

    return stmt;
}

static node_t*
sema_stmt(state_t *self, node_t *stmt);

static node_t*
sema_compound(state_t *self, node_t *stmt)
{
    node_t *stmts = stmt->compound->data;

    for (size_t i = 0; i < stmt->compound->length; i++) {
        memcpy(stmts + i, sema_stmt(self, stmts + i), sizeof(node_t));
    }

    return stmt;
}

static node_t*
sema_stmt(state_t *self, node_t *stmt)
{
    state_t nest;
    node_t *node;
    switch (stmt->kind) {
    case NODE_COMPOUND:
        nest = new_state(self);
        return sema_compound(&nest, stmt);
    case NODE_VAR:
        node = sema_var(self, stmt);
        add_decl(self, node);
        return node;
    case NODE_ASSIGN:
        return sema_assign(self, stmt);
    case NODE_RETURN:
        return sema_return(self, stmt);
    default:
        ERRF(self, "sema_stmt unknown kind %d\n", stmt->kind);
        return stmt;
    }
}

#define RESOLVE(state, node) node = flatten_type(state, node)

static node_t*
sema_global_func(state_t *self, node_t *decl)
{
    state_t nest = new_state(self);

    node_t *res = decl->decl.func.result;
    node_t *result = res ? flatten_type(self, res) : get_voidtype(self);

    decl->decl.func.result = result;

    for (size_t i = 0; i < decl->decl.func.params->length; i++) {
        RESOLVE(self, (decl->decl.func.params->data + i)->decl.param);
        add_decl(&nest, (decl->decl.func.params->data + i));
    }

    nest.ret = result;

    decl->decl.func.body = sema_stmt(&nest, decl->decl.func.body);

    return decl;
}

static node_t*
sema_decl(state_t *self, node_t *decl)
{
    switch (decl->kind) {
    case NODE_FUNC: 
        return sema_global_func(self, decl);
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
        memcpy(p + i, sema_decl(self, p + i), sizeof(node_t));
    }
}
