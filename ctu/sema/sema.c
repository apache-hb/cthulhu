#include "sema.h"

#include "ctu/util/report.h"
#include "ctu/util/util.h"
#include "ctu/util/str.h"

#include "ctu/ast/compile.h"

#include <errno.h>

typedef struct sema_t {
    struct sema_t *parent;

    /** 
     * @var map_t<string, sema_t*> 
     * 
     * map of imports to their respective scope
     */
    map_t *imports;

    /** 
     * @var map_t<string, node_t*> 
     * 
     * variables in the current scope
     */
    map_t *vars;

    /** 
     * @var map_t<string, node_t*> 
     * 
     * type declarations
     */
    map_t *types;

    /** 
     * @var map_t<string, node_t*> 
     * 
     * function declarations
     */
    map_t *funcs;


    /**
     * the return type of the current function
     */
    type_t *result;

    /**
     * counters for certain types of nodes
     */

    /* the local index for the current function */
    size_t locals;
} sema_t;

/**
 * builtin types
 */
static map_t *builtins = NULL;

/**
 * all parsed files
 * kept in a map so we never open a file twice
 */
static map_t *files = NULL;

/**
 * total number of strings
 */
static size_t strings = 0;

static size_t locals = 0;

static list_t *funcs = NULL;
static list_t *vars = NULL;
static list_t *types = NULL;

static sema_t *begin_file(node_t *inc, node_t *root, const char *path);

static sema_t *new_sema(sema_t *parent) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));

    if (parent) {
        sema->imports = parent->imports;
        sema->result = parent->result;
    } else {
        sema->imports = new_map(8);
        sema->result = NULL;
    }

    sema->parent = parent;
    sema->vars = new_map(8);
    sema->types = new_map(8);
    sema->funcs = new_map(8);
    sema->locals = 0;

    return sema;
}

static void mark_local(node_t *node) {
    node->local = locals++;
}

static char *path_of(list_t *it) {
    char *joined = str_join("/", (const char**)it->data, it->len);
    char *out = format("%s.ct", joined);
    ctu_free(joined);

    return out;
}

static void add_import(sema_t *sema, node_t *it) {
    /**
     * compile the imported file
     */
    char *path = path_of(it->path);
    sema_t *other = begin_file(it, NULL, path);

    /**
     * then add it to the current scope
     * currently we add it by taking the last element
     * of the path and using that as the key.
     */
    map_put(sema->imports, list_last(it->path), other);
}

static void put_unique(map_t *dst, const char *key, node_t *node) {
    if (map_get(dst, key)) {
        reportf(LEVEL_ERROR, node, "redeclaration of `%s`", key);
    } else {
        map_put(dst, key, node);
    }
}

static void put_decl_unique(sema_t *sema, node_t *node) {
    const char *key = get_decl_name(node);
    if (map_get(sema->vars, key) != NULL || map_get(sema->funcs, key) != NULL) {
        reportf(LEVEL_ERROR, node, "redeclaration of `%s`", key);
    } else {
        switch (node->kind) {
        case AST_DECL_VAR: 
            map_put(sema->vars, key, node);
            break;
        case AST_DECL_FUNC:
            map_put(sema->funcs, key, node);
            break;

        default:
            assert("put_decl_unique invalid node %d", node->kind);
            break;
        }
    }
}

static void add_decl(sema_t *sema, node_t *it) {
    const char *name = get_decl_name(it);

    switch (it->kind) {
    case AST_DECL_VAR: case AST_DECL_FUNC:
        put_decl_unique(sema, it);
        break;

    case AST_DECL_STRUCT: 
        put_unique(sema->types, name, it);
        break;

    default:
        assert("unknown add_decl kind %d", it->type);
        break;
    }
}

static void add_discard(map_t *dst, node_t *it) {
    const char *name = get_decl_name(it);
    if (!is_discard_name(name)) {
        map_put(dst, name, it);
    }
}

static void build_var(sema_t *sema, node_t *it);

#include "type.c"
#include "expr.c"
#include "stmt.c"

static void build_type(sema_t *sema, node_t *it) {
    switch (it->kind) {
    case AST_DECL_STRUCT:
        build_struct(sema, it);
        break;

    default:
        assert("unknown type %d", it->kind);
        break;
    }
}

static void build_var(sema_t *sema, node_t *it) {
    ASSERT(it->init || it->type)("var must be partialy initialized");
    
    type_t *type = NULL;
    type_t *init = NULL;

    type_t *out = NULL;

    if (it->init) {
        init = query_expr(sema, it->init);
        out = init;
    }

    if (it->type) {
        type = query_type(sema, it->type);
        out = type;
    }

    /**
     * variables are lvalues
     */

    out = make_lvalue(out);
    
    if (it->mut) {
        out = make_mut(out);
    }

    if (type != NULL && init != NULL) {
        if (!type_can_become_implicit(&it, type, init)) {
            reportf(LEVEL_ERROR, it, "incompatible types for initialization of variable `%s`", it->name);
        }
    }

    connect_type(it, out);
}

static void build_params(sema_t *sema, list_t *params) {
    for (size_t i = 0; i < list_len(params); i++) {
        node_t *param = list_at(params, i);
        type_t *type = query_type(sema, param);

        add_discard(sema->vars, param);
        mark_local(param);

        if (is_void(type)) {
            reportf(LEVEL_ERROR, param, "void type not allowed as parameter");
        }
    }
}

static void build_func(sema_t *sema, node_t *it) {
    sema_t *nest = new_sema(sema);
    build_params(nest, it->params);

    nest->result = query_type(sema, it->result);

    build_stmts(nest, it->body);
    it->locals = locals;
    locals = 0;
}

/**
 * `inc` is the import that caused this file to be parsed
 * parse a file at `path`
 * if root is null then this function will `ctu_open` 
 * the path and attempt to parse it. otherwise it will use it
 */
static sema_t *begin_file(node_t *inc, node_t *root, const char *path) {
    /**
     * if the files already parsed
     * then reuse it
     */
    sema_t *sema = map_get(files, path);
    if (sema != NULL) {
        return sema;
    }
    
    /**
     * if we havent been given a root node
     * then open the file and parse it
     */
    if (root == NULL) {
        FILE *fd = ctu_open(path, "rb");
        if (!fd) {
            reportf(LEVEL_ERROR, inc, "failed to open `%s` due to `%s`", path, strerror(errno));
            return NULL;
        }

        root = compile_file(path, fd, NULL);
        if (root == NULL) {
            return NULL;
        }
    }

    /**
     * now typecheck the ast
     */
    sema = new_sema(NULL);
    map_put(files, path, sema);

    /**
     * first add all its imports
     */
    list_t *imports = root->imports;
    for (size_t i = 0; i < list_len(imports); i++) {
        add_import(sema, list_at(imports, i));
    }

    list_t *decls = root->decls;

    /**
     * we add all the decls first for order independant
     * lookup
     */

    for (size_t i = 0; i < list_len(decls); i++) {
        node_t *decl = list_at(decls, i);
        add_decl(sema, decl);
    }

    /**
     * now add all declarations
     */

    return sema;
}

static void build_type_wrap(const char *id, void *data, void *arg) {
    (void)id;
    build_type(data, arg);
    list_push(types, data);
}

static void build_var_wrap(const char *id, void *data, void *arg) {
    (void)id;
    build_var(arg, data);
    list_push(vars, data);
}

static void build_func_wrap(const char *id, void *data, void *arg) {
    (void)id;
    build_func(arg, data);
    list_push(funcs, data);
}

static void build_file(sema_t *sema) {
    map_iter(sema->types, build_type_wrap, sema);
    map_iter(sema->vars, build_var_wrap, sema);
    map_iter(sema->funcs, build_func_wrap, sema);
}

static void build_file_wrap(const char *id, void *data, void *arg) {
    (void)arg;
    (void)id;
    build_file(data);
}

unit_t typecheck(node_t *root) {
    funcs = new_list(NULL);
    vars = new_list(NULL);
    types = new_list(NULL);

    begin_file(NULL, root, root->scanner->path);

    map_iter(files, build_file_wrap, NULL);

    unit_t unit = { funcs, vars, types, strings };

    return unit;
}

static void add_builtin(type_t *type) {
    map_put(builtins, type->node->name, type);
}

void sema_init(void) {
    files = new_map(8);
    builtins = new_map(32);

    for (int i = 0; i < INTEGER_END; i++) {
        add_builtin(get_int_type(false, i));
        add_builtin(get_int_type(true, i));
    }

    add_builtin(STRING_TYPE);
    add_builtin(BOOL_TYPE);
    add_builtin(VOID_TYPE);
}

#if 0

#include "ctu/util/report.h"
#include "ctu/util/util.h"
#include "ctu/util/str.h"

#include "ctu/debug/type.h"
#include "ctu/debug/ast.h"

#include "ctu/ast/compile.h"

#include <gmp.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

/**
 * structures
 */

typedef struct sema_t {
    struct sema_t *parent;

    /**
     * a map of the imported name to the full path
     */

    /** @var map_t<const char*, const char*> */
    map_t *imports;

    /**
     * all types in the current scope
     */
    /** @var map_t<const char*, node_t*> */
    map_t *decls;

    /**
     * all variables in the current scope
     */
    /** @var map_t<const char*, node_t*> */
    map_t *vars;

    type_t *result; /* return type of current function */
} sema_t;

size_t locals;
size_t strings;

/**
 * all files
 * @var map_t<const char*, sema_t*>
 */
map_t *files;

/**
 * constants
 */

static sema_t *ROOT_SEMA = NULL;

/**
 * builders
 */

static sema_t *base_sema(sema_t *parent, map_t *imports, size_t size) {
    sema_t *sema = ctu_malloc(sizeof(sema_t));
    sema->parent = parent;
    sema->decls = new_map(size);
    sema->vars = new_map(size);
    sema->result = NULL;
    sema->imports = imports;
    return sema;
}

static sema_t *new_sema(sema_t *parent) {
    return base_sema(parent, NULL, 32);
}

static void free_sema(sema_t *sema) {
    ctu_free(sema);
}

/**
 * declaration managment
 */

static type_t *return_type(sema_t *sema) {
    type_t *result = sema->result;
    
    if (result) {
        return result;
    }

    if (sema->parent) {
        return return_type(sema->parent);
    }

    return VOID_TYPE;
}

static void add_decl(sema_t *sema, node_t *decl) {
    map_put(sema->decls, get_resolved_name(decl), decl);
}

static node_t *get_decl(sema_t *sema, const char *name) {
    node_t *decl = map_get(sema->decls, name);
    if (decl) {
        mark_used(decl);
        return decl;
    }

    /**
     * recurse if we can
     */
    if (sema->parent) {
        return get_decl(sema->parent, name);
    }

    /**
     * the decl doesnt exist
     */
    return NULL;
}

static void add_decl_unique(sema_t *sema, node_t *node) {
    const char *name = get_resolved_name(node);
    node_t *previous = get_decl(sema, name);

    /**
     * make sure we arent shadowing an existing declaration
     */
    if (previous != NULL) {
        reportf(LEVEL_ERROR, node, "`%s` has already been declared", name);
    }

    add_decl(sema, node);
}

static void add_decl_global(sema_t *sema, node_t *decl) {
    if (is_discard_name(get_decl_name(decl))) {
        reportf(LEVEL_ERROR, decl, "may not discard declaration");
        return;
    }

    add_decl_unique(sema, decl);
}

static void mark_string(node_t *str) {
    str->local = strings++;
}

static size_t reset_strings(void) {
    size_t num = strings;
    strings = 0;
    return num;
}

static void mark_local(node_t *decl) {
    decl->local = locals++;
}

static size_t reset_locals(void) {
    size_t num = locals;
    locals = 0;
    return num;
}

static void add_local(sema_t *sema, node_t *decl, bool add) {
    if (add) {
        add_decl_unique(sema, decl);
    }

    mark_local(decl);
}

static void add_discardable_local(sema_t *sema, node_t *decl) {
    bool discard = is_discard_name(get_decl_name(decl));
    add_local(sema, decl, !discard);
}

static bool is_local(node_t *node) {
    return node->kind == AST_DECL_VAR 
        || node->kind == AST_DECL_PARAM;
}

static type_t *query_symbol(sema_t *sema, node_t *symbol) {
    const char *name = list_last(symbol->ident);
    node_t *origin = get_decl(sema, name);

    if (origin == NULL) {
        return new_unresolved(symbol);
    }

    /**
     * if the variable is a local variable
     * then propogate what its index is
     * for the ir gen
     */
    symbol->local = is_local(origin)
        ? origin->local
        : NOT_LOCAL;

    return get_type(origin);
}

static type_t *resolve_symbol(sema_t *sema, node_t *symbol) {
    type_t *type = query_symbol(sema, symbol);

    if (is_unresolved(type)) {
        reportf(LEVEL_ERROR, symbol, "cannot resolve `%s` to a type", list_last(symbol->ident));
    }

    connect_type(symbol, type);

    return type;
}

static type_t *resolve_typename(sema_t *sema, node_t *node) {
    type_t *type = raw_type(node);
    if (type) {
        return type;
    }

    return resolve_symbol(sema, node);
}

static type_t *resolve_type(sema_t *sema, node_t *node) {
    if (node->kind == AST_PTR) {
        return new_pointer(node, resolve_type(sema, node->ptr));
    } else if (node->kind == AST_MUT) {
        type_t *type = resolve_type(sema, node->next);
        return set_mut(type, true);
    } else {
        return resolve_typename(sema, node);
    }
}

static node_t *implicit_cast(node_t *original, type_t *to) {
    node_t *cast = ast_cast(original->scanner, original->where, original, NULL);

    connect_type(cast, to);

    return make_implicit(cast);
}

/**
 * comparison
 */

static void check_sign_conversion(node_t **node, type_t *to, type_t *from, bool implicit) {
    if (is_signed(to) != is_signed(from) && implicit) {
        reportf(LEVEL_WARNING, *node, "types do not have the same sign, conversion may be lossy");
        *node = implicit_cast(*node, to);
    }
}

static bool convertible_to(
    node_t **node,
    type_t *to, type_t *from,
    bool implicit
) {
    /* if the types are the same then obviously convertible */
    if (to == from) {
        return true;
    }

    if (is_void(to)) {
        return is_void(from);
    }
    
    if (is_integer(to) && is_integer(from)) {
        check_sign_conversion(node, to, from, implicit);

        return true;
    }

    if (is_boolean(to)) {
        if (is_boolean(from)) {
            return true;
        }

        if (is_integer(from)) {
            if (implicit) {
                reportid_t report = reportf(LEVEL_WARNING, *node, "implicit integer to boolean conversion");
                report_underline(report, "conversion happens here");
                report_note(report, "add an explicit cast to silence this warning");
                node_t *temp = implicit_cast(*node, to);
                *node = temp;
            }

            return true;
        }
    }

    if (is_pointer(to) && is_pointer(from)) {
        if (to->ptr->mut && !from->ptr->mut) {
            reportf(LEVEL_ERROR, *node, "cannot discard const from pointer type");
        }

        return convertible_to(node, to->ptr, from->ptr, implicit);
    }

    if (is_struct(to) && is_struct(from)) {
        /* this wont work once imports are a thing */
        return strcmp(to->name, from->name) == 0;
    }

    return false;
}

static bool implicit_convertible_to(node_t **node, type_t *to, type_t *from) {
    return convertible_to(node, to, from, true);
}

static bool explicit_convertible_to(type_t *to, type_t *from) {
    return convertible_to(NULL, to, from, false);
}

/**
 * typechecking
 */

static void typecheck_stmt(sema_t *sema, node_t *stmt);
static type_t *typecheck_expr(sema_t *sema, node_t *expr);
static type_t *typecheck_decl(sema_t *sema, node_t *decl);

static type_t *typecheck_return(sema_t *sema, node_t *stmt) {
    node_t *expr = stmt->expr;

    type_t *result = expr == NULL 
        ? VOID_TYPE
        : typecheck_expr(sema, expr);

    if (result == VOID_TYPE && expr != NULL) {
        reportf(LEVEL_ERROR, stmt, "cannot explicitly return void");
    }

    type_t *expect = return_type(sema);

    if (!implicit_convertible_to(&stmt->expr, expect, result)) {
        reportf(LEVEL_ERROR, stmt, "incorrect return type");
    }
    
    result = stmt->expr == NULL
        ? VOID_TYPE
        : get_type(stmt->expr);

    return result;
}

static void typecheck_stmts(sema_t *sema, node_t *stmts) {
    list_t *list = get_stmts(stmts);
    
    for (size_t i = 0; i < list_len(list); i++) {
        typecheck_stmt(sema, list_at(list, i));
    }
}

static type_t *get_digit_type(node_t *digit) {
    return get_int_type(digit->sign, digit->integer);
}

static type_t *get_bool_type(void) {
    return BOOL_TYPE;
}

static type_t *typecheck_call(sema_t *sema, node_t *node) {
    type_t *body = typecheck_expr(sema, node->expr);

    if (!is_callable(body)) {
        reportf(LEVEL_ERROR, node, "uncallable type");
        return new_poison(node, "uncallable type");
    }

    size_t params = typelist_len(body->args);
    size_t args = list_len(node->args);

    if (params != args) {
        reportf(LEVEL_ERROR, node, "incorrect number of parameters");
        return body->result;
    }

    for (size_t i = 0; i < params; i++) {
        node_t *arg = list_at(node->args, i);
        type_t *to = typelist_at(body->args, i);
        type_t *from = typecheck_expr(sema, arg);
    
        if (!implicit_convertible_to(&arg, to, from)) {
            reportf(LEVEL_ERROR, arg, "incompatible argument type");
        }
    }

    return body->result;
}

static type_t *typecheck_func(sema_t *sema, node_t *decl) {
    type_t *result = resolve_type(sema, decl->result);
    size_t len = list_len(decl->params);
    types_t *args = new_typelist(len);

    for (size_t i = 0; i < len; i++) {
        node_t *param = list_at(decl->params, i);
        type_t *arg = typecheck_decl(sema, param);
        typelist_put(args, i, arg);
    }

    return new_callable(decl, args, result);
}

static void check_binary_cmp(node_t* node, type_t *lhs, type_t *rhs) {
    if (!is_integer(lhs) || !is_integer(rhs)) {
        reportf(LEVEL_ERROR, node, "both sides of comparison operation must be integral");
    }
}

static bool check_binary_math(node_t* node, type_t *lhs, type_t *rhs) {
    if (!is_integer(lhs) || !is_integer(rhs)) {
        reportf(LEVEL_ERROR, node, "both sides of math operation must be integral");
        return false;
    }
    return true;
}

static void check_binary_equality(node_t* node, type_t *lhs, type_t *rhs) {
    if (is_integer(lhs) && is_integer(rhs)) {
        return;
    }

    if (is_boolean(lhs) && is_boolean(rhs)) {
        return;
    }

    reportf(LEVEL_ERROR, node, "incompatible comparison operands");
}

// get the largest integer we need
static type_t *get_binary_math_type(type_t *lhs, type_t *rhs) {
    integer_t it = MAX(get_integer_kind(lhs), get_integer_kind(rhs));

    return get_int_type(is_signed(lhs) || is_signed(rhs), it);
}

static type_t *typecheck_binary(sema_t *sema, node_t *expr) {
    type_t *lhs = typecheck_expr(sema, expr->lhs);
    type_t *rhs = typecheck_expr(sema, expr->rhs);
    binary_t op = expr->binary;
    type_t *result = BOOL_TYPE;

    if (is_comparison_op(op)) {
        check_binary_cmp(expr, lhs, rhs);
    } else if (is_equality_op(op)) {
        check_binary_equality(expr, lhs, rhs);
    } else if (is_math_op(op)) {
        if (check_binary_math(expr, lhs, rhs)) {
            result = get_binary_math_type(lhs, rhs);
        }
    } else {
            result = new_poison(expr, "unknown operation");
        }

    return result;
}

static bool is_lvalue(node_t *expr) {
    return expr->kind == AST_SYMBOL;
}

static bool is_assignable(type_t *type, node_t *expr) {
    if (is_deref(expr))
        return is_assignable(type, expr->expr);

    if (is_access(expr) || expr->kind == AST_SYMBOL)
        return !is_const(type);

    return false;
}

static type_t *typecheck_unary(sema_t *sema, node_t *expr) {
    type_t *type = typecheck_expr(sema, expr->expr);
    unary_t op = expr->unary;

    switch (op) {
    case UNARY_ABS: case UNARY_NEG:
        if (!is_integer(type)) {
            reportf(LEVEL_ERROR, expr, "unary operation requires integral");
        }
        if (!is_signed(type) && op == UNARY_NEG) {
            reportf(LEVEL_WARNING, expr, "unary negation on an unsigned type");
        }
        break;

    case UNARY_REF:
        if (!is_lvalue(expr->expr)) {
            reportf(LEVEL_ERROR, expr, "cannot take a reference to a non-lvalue");
        } else {
            type = new_pointer(expr, type);
        }
        break;
    case UNARY_DEREF:
        if (!is_pointer(type)) {
            reportf(LEVEL_ERROR, expr, "cannot dereference a type that isnt a pointer");
        } else {
            type = type->ptr;
        }
        break;
    case UNARY_TRY:
        reportf(LEVEL_INTERNAL, expr, "unimplemented unary operation");
        break;
    }

    return type;
}

static void typecheck_branch(sema_t *sema, node_t *stmt) {
    if (stmt->cond) {
        type_t *cond = typecheck_expr(sema, stmt->cond);
        if (!implicit_convertible_to(&stmt->cond, BOOL_TYPE, cond)) {
            reportf(LEVEL_ERROR, stmt->cond, "cannot branch on a non-boolean type");
        }
    }

    typecheck_stmt(sema, stmt->branch);

    if (stmt->next) {
        typecheck_branch(sema, stmt->next);
    }
}

static type_t *typecheck_cast(sema_t *sema, node_t *cast) {
    type_t *origin = typecheck_expr(sema, cast->expr);
    type_t *target = resolve_type(sema, cast->cast);

    if (!explicit_convertible_to(target, origin)) {
        reportf(LEVEL_ERROR, cast, "cannot perform explicit conversion");
    }

    return target;
}

static type_t *get_field(node_t *node, type_t *type, const char *name) {
    for (size_t i = 0; i < type->fields.size; i++) {
        field_t field = type->fields.fields[i];
        if (strcmp(field.name, name) == 0) {
            return field.type;
        }
    }

    reportf(LEVEL_ERROR, node, "no field `%s` in struct `%s`", name, type->name);
    return new_poison(node, "unknown field");
}

static type_t *typecheck_access(sema_t *sema, node_t *access) { 
    type_t *body = typecheck_expr(sema, access->target);

    if (is_pointer(body) && !access->indirect) { 
        reportf(LEVEL_ERROR, access, "cannot access a pointer without indirection");
    }

    type_t *inner = is_pointer(body)
        ? body->ptr
        : body;

    if (!is_struct(inner)) {
        reportf(LEVEL_ERROR, access, "cannot access a non-struct type");
        return new_poison(access, "access to a non-struct type");
    }

    return get_field(access, inner, access->field);
}

static type_t *get_string_type(node_t *str) {
    mark_string(str);
    return STRING_TYPE;
}

static type_t *typecheck_expr(sema_t *sema, node_t *expr) {
    type_t *type = raw_type(expr);

    if (type) {
        return type;
    }

    switch (expr->kind) {
    case AST_DIGIT: 
        type = get_digit_type(expr);
        break;

    case AST_BOOL:
        type = get_bool_type();
        break;

    case AST_STRING:
        type = get_string_type(expr);
        break;

    case AST_BINARY:
        type = typecheck_binary(sema, expr);
        break;

    case AST_UNARY:
        type = typecheck_unary(sema, expr);
        break;

    case AST_SYMBOL:
        type = resolve_symbol(sema, expr);
        break;

    case AST_CALL:
        type = typecheck_call(sema, expr);
        break;

    case AST_CAST:
        type = typecheck_cast(sema, expr);
        break;

    case AST_ACCESS:
        type = typecheck_access(sema, expr);
        break;

    default:
        reportf(LEVEL_INTERNAL, expr, "unimplemented expression typecheck");
        type = new_poison(expr, "unimplemented expression typecheck");
        break;
    }

    connect_type(expr, type);

    return type;
}

static type_t *typecheck_var(sema_t *sema, node_t *decl) {
    type_t *type = NULL;
    type_t *init = NULL;

    type_t *var = NULL;

    if (decl->type) {
        type = resolve_type(sema, decl->type);
        var = type;
    }

    if (decl->init) {
        init = typecheck_expr(sema, decl->init);
        var = init;
    }

    if (type && init) {
        if (!implicit_convertible_to(&decl, type, init)) {
            reportf(LEVEL_ERROR, decl, "variable type and initializer are incompatible");
        }
    }

    var = set_mut(var, decl->mut);

    connect_type(decl, var);

    return var;
}

static void typecheck_assign(sema_t *sema, node_t *decl) {
    type_t *dst = typecheck_expr(sema, decl->dst);
    type_t *src = typecheck_expr(sema, decl->src);

    if (!is_assignable(dst, decl->dst)) {
        reportf(LEVEL_ERROR, decl, "cannot assign to a non-lvalue");
    }

    if (!implicit_convertible_to(&decl, dst, src)) {
        reportf(LEVEL_ERROR, decl, "cannot assign unrelated types");
    }
}

static void typecheck_while(sema_t *sema, node_t *decl) {
    type_t *cond = typecheck_expr(sema, decl->cond);
    typecheck_stmt(sema, decl->next);

    if (!implicit_convertible_to(&decl->cond, BOOL_TYPE, cond)) {
        reportf(LEVEL_ERROR, decl->cond, "cannot loop on a non-boolean condition");
    }
}

static bool struct_contains_struct(type_t *type, type_t *member) {
    fields_t fields = type->fields;
    for (size_t i = 0; i < fields.size; i++) {
        field_t field = fields.fields[i];
        type_t *it = field.type;
        if (is_struct(it)) {
            if (strcmp(it->name, member->name) == 0) {
                return true;
            }

            if (struct_contains_struct(it, member)) {
                return true;
            }
        }
    }
    return false;
}

static void add_field(sema_t *sema, size_t at, type_t *record, node_t *field) {
    const char *name = get_field_name(field);

    for (size_t i = 0; i < at; i++) {
        field_t it = record->fields.fields[i];
        const char *other = it.name;

        if (!is_discard_name(name) && strcmp(name, other) == 0) { 
            reportf(LEVEL_ERROR, field, "duplicate field `%s` in struct", name);
        }
    }

    field_t it = { name, resolve_type(sema, field->type) };

    record->fields.fields[at] = it;
}   

static type_t *begin_struct(node_t *decl) { 
    const char *name = get_decl_name(decl);

    return new_struct(decl, name);
}

static void build_struct(sema_t *sema, node_t *decl) {
    type_t *result = get_type(decl);

    list_t *fields = decl->fields;
    size_t len = list_len(fields);

    resize_struct(result, len);

    for (size_t i = 0; i < len; i++) {
        node_t *field = list_at(fields, i);
        add_field(sema, i, result, field);
    }
}

static type_t *typecheck_struct(node_t *decl) {
    return get_type(decl);
}

static void validate_struct(node_t *decl) {
    type_t *record = get_type(decl);
    fields_t fields = record->fields;
    size_t len = fields.size;
    for (size_t i = 0; i < len; i++) {
        field_t field = fields.fields[i];
        if (is_struct(field.type)) {
            if (struct_contains_struct(field.type, record)) {
                reportf(LEVEL_ERROR, decl, "struct `%s` contains itself recursivley", record->name);
                record->invalid = true;
            }
        }
    }
}

static type_t *typecheck_decl(sema_t *sema, node_t *decl) {
    type_t *type = raw_type(decl);

    if (type) {
        return type;
    }

    switch (decl->kind) {
    case AST_DECL_PARAM:
        type = resolve_type(sema, decl->type);
        break;

    case AST_DECL_FUNC:
        type = typecheck_func(sema, decl);
        break;

    case AST_DECL_VAR:
        type = typecheck_var(sema, decl);
        add_discardable_local(sema, decl);
        return type;

    case AST_DECL_STRUCT:
        type = typecheck_struct(decl);
        break;

    default:
        reportf(LEVEL_INTERNAL, decl, "unimplemented declaration typecheck");
        type = new_poison(decl, "unimplemented declaration typecheck");
        break;
    }

    connect_type(decl, type);

    return type;
}

static void typecheck_stmt(sema_t *sema, node_t *stmt) {
    type_t *type = VOID_TYPE;
    sema_t *nest;

    switch (stmt->kind) {
    case AST_RETURN:
        type = typecheck_return(sema, stmt);
        break;

    case AST_STMTS:
        nest = base_sema(sema, NULL, 8);
        typecheck_stmts(nest, stmt);
        free_sema(nest);
        break;

    case AST_BRANCH:
        typecheck_branch(sema, stmt);
        break;

    case AST_DECL_VAR:
        type = typecheck_decl(sema, stmt);
        break;

    case AST_DIGIT: case AST_UNARY: 
    case AST_BINARY: case AST_CALL:
        type = typecheck_expr(sema, stmt);
        if (!is_void(type)) {
            reportf(LEVEL_WARNING, stmt, "discarding value of expression");
        }
        break;

    case AST_ASSIGN:
        typecheck_assign(sema, stmt);
        break;

    case AST_WHILE:
        typecheck_while(sema, stmt);
        break;

    default:
        reportf(LEVEL_INTERNAL, stmt, "unimplement statement typecheck");
        break;
    }

    connect_type(stmt, type);
}

static void validate_params(sema_t *sema, list_t *params) {
    size_t len = list_len(params);
    for (size_t i = 0; i < len; i++) {
        node_t *param = list_at(params, i);
        type_t *type = typecheck_decl(sema, param);
        
        add_discardable_local(sema, param);

        if (is_void(type)) {
            reportf(LEVEL_ERROR, param, "parameter cannot have void type");
        }
    }
}

static void validate_function(sema_t *sema, node_t *func) {
    sema_t *nest = new_sema(sema);
    validate_params(nest, func->params);
    sema->result = resolve_type(sema, func->result);
    
    typecheck_stmts(nest, func->body);

    free_sema(nest);
}

static void add_all_decls(sema_t *sema, list_t *decls) {
    size_t len = list_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = list_at(decls, i);
        switch (decl->kind) {
        case AST_DECL_FUNC: typecheck_func(sema, decl); break;
        case AST_DECL_VAR: typecheck_var(sema, decl); break;
        case AST_DECL_STRUCT: begin_struct(decl); break;
        default: assert("unknown decl type %d", decl->kind);
        }
        add_decl_global(sema, decl);
    }

    for (size_t i = 0; i < len; i++) {
        node_t *decl = list_at(decls, i);
        if (decl->kind == AST_DECL_STRUCT) {
            build_struct(sema, decl);
        }
    }
}

static void validate_var(sema_t *sema, node_t *var) {
    typecheck_decl(sema, var);
}

static void typecheck_all_decls(sema_t *sema, list_t *decls) {
    size_t len = list_len(decls);

    for (size_t i = 0; i < len; i++) {
        node_t *decl = list_at(decls, i);
        switch (decl->kind) {
        case AST_DECL_VAR: 
            validate_var(sema, decl); 
            break;
        case AST_DECL_FUNC: 
            validate_function(sema, decl);
            decl->locals = reset_locals();
            break;
        case AST_DECL_STRUCT:
            validate_struct(decl);
            break;
        default: 
            assert("unknown decl type %d", decl->kind);
            break;
        }
    }
}

static void add_builtin(type_t *type) {
    add_decl(ROOT_SEMA, type->node);
}

static void add_import(sema_t *sema, sema_t *other, node_t *it, const char *name) {
    if (map_get(sema->imports, name) != NULL) {
        reportf(LEVEL_ERROR, it, "import %s already declared", name);
    } else {
        map_put(sema->imports, name, other);
    }
}

static char *path_of(list_t *parts) {
    char *seps = str_join("/", (const char**)parts->data, parts->len);
    return format("%s.ct", seps);
}

static sema_t *import_file(node_t *inc, const char *path);

static void add_imports(sema_t *sema, list_t *imports) {
    for (size_t i = 0; i < list_len(imports); i++) {
        node_t *it = list_at(imports, i);
        list_t *parts = it->path;
        char *name = list_last(parts);

        const char *path = path_of(parts);

        sema_t *other = import_file(it, path);
        
        if (other == NULL) {
            add_import(sema, other, it, name);
        }
    }
}

static sema_t *typecheck_file(node_t *root, const char *path) {
    list_t *decls = root->decls;

    sema_t *sema = base_sema(ROOT_SEMA, new_map(8), 256);

    /**
     * register all decls first
     * so recursive imports can find them
     */
    add_all_decls(sema, decls);

    map_put(files, path, sema);

    /**
     * then add all imports and process them
     */
    add_imports(sema, root->imports);

    /**
     * then finally typecheck everything
     * in the current file
     */
    typecheck_all_decls(sema, decls);

    return sema;
}

static sema_t *import_file(node_t *inc, const char *path) {
    printf("get: %s\n", path);
    sema_t *other = map_get(files, path);
    if (other != NULL) {
        return other;
    }

    FILE *stream = ctu_open(path, "r");
    if (stream == NULL) {
        reportf(LEVEL_ERROR, inc, "failed to open file `%s` due to `%s`", path, strerror(errno));
        return NULL;
    }

    node_t *root = compile_file(path, stream, NULL);

    if (report_end(format("compiling `%s`", path))) {
        reportf(LEVEL_ERROR, inc, "failed to compile file `%s`", path);
        return NULL;
    }

    sema_t *res = typecheck_file(root, path);

    return res;
}

/**
 * external api
 */

unit_t typecheck(node_t *root) {
    reset_strings();

    typecheck_file(root, root->scanner->path);

    unit_t unit = { new_list(NULL), 0 };

    return unit;
}

void sema_init(void) {
    files = new_map(16);
    ROOT_SEMA = new_sema(NULL);

    for (int i = 0; i < INTEGER_END; i++) {
        add_builtin(get_int_type(false, i));
        add_builtin(get_int_type(true, i));
    }

    add_builtin(STRING_TYPE);
    add_builtin(BOOL_TYPE);
    add_builtin(VOID_TYPE);
}
#endif
