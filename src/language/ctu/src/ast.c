// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/ast.h"

#include "std/vector.h"

#include "arena/arena.h"

#include "cthulhu/broker/scan.h"

static ctu_t *ctu_new(scan_t *scan, where_t where, ctu_kind_t kind)
{
    arena_t *arena = ctx_get_ast_arena(scan);

    ctu_t *self = ARENA_MALLOC(sizeof(ctu_t), "ctu", scan, arena);
    self->kind = kind;
    self->node = node_new(scan, where);

    ARENA_IDENTIFY(self->node, "node", self, arena);

    return self;
}

static ctu_t *ctu_decl(scan_t *scan, where_t where, ctu_kind_t kind, char *name, bool exported)
{
    ctu_t *self = ctu_new(scan, where, kind);
    self->name = name;
    self->exported = exported;
    self->attribs = &kEmptyVector;
    return self;
}

ctu_t *ctu_module(scan_t *scan, where_t where, const vector_t *modspec, const vector_t *imports, const vector_t *decls)
{
    ctu_t *ast = ctu_new(scan, where, eCtuModule);
    ast->modspec = modspec;
    ast->imports = imports;
    ast->decls = decls;
    return ast;
}

ctu_t *ctu_import(scan_t *scan, where_t where, vector_t *path, char *name)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuImport, name, false);
    ast->import_path = path;
    return ast;
}

/* attribs */

ctu_t *ctu_attrib(scan_t *scan, where_t where, const vector_t *path, const vector_t *args)
{
    ctu_t *ast = ctu_new(scan, where, eCtuAttrib);
    ast->attrib_path = path;
    ast->attrib_args = args;
    return ast;
}

ctu_t *ctu_apply(ctu_t *decl, const vector_t *attribs)
{
    decl->attribs = attribs;
    return decl;
}

/* stmts */

ctu_t *ctu_stmt_list(scan_t *scan, where_t where, vector_t *stmts)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtList);
    ast->stmts = stmts;
    return ast;
}

ctu_t *ctu_stmt_local(scan_t *scan, where_t where, bool mutable, char *name, ctu_t *type, ctu_t *value)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuStmtLocal, name, false);
    ast->mut = mutable;
    ast->type = type;
    ast->value = value;
    return ast;
}

ctu_t *ctu_stmt_return(scan_t *scan, where_t where, ctu_t *value)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtReturn);
    ast->result = value;
    return ast;
}

ctu_t *ctu_stmt_while(scan_t *scan, where_t where, char *name, ctu_t *cond, ctu_t *then, ctu_t *other)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuStmtWhile, name, false);
    ast->cond = cond;
    ast->then = then;
    ast->other = other;
    return ast;
}

ctu_t *ctu_stmt_assign(scan_t *scan, where_t where, ctu_t *dst, ctu_t *src)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtAssign);
    ast->dst = dst;
    ast->src = src;
    return ast;
}

ctu_t *ctu_stmt_break(scan_t *scan, where_t where, char *label)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtBreak);
    ast->label = label;
    return ast;
}

ctu_t *ctu_stmt_continue(scan_t *scan, where_t where, char *label)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtContinue);
    ast->label = label;
    return ast;
}

ctu_t *ctu_stmt_branch(scan_t *scan, where_t where, ctu_t *cond, ctu_t *then, ctu_t *other)
{
    ctu_t *ast = ctu_new(scan, where, eCtuStmtBranch);
    ast->cond = cond;
    ast->then = then;
    ast->other = other;
    return ast;
}

/* exprs */

ctu_t *ctu_expr_int(scan_t *scan, where_t where, mpz_t value)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprInt);
    mpz_init_set(ast->int_value, value);
    return ast;
}

ctu_t *ctu_expr_bool(scan_t *scan, where_t where, bool value)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprBool);
    ast->bool_value = value;
    return ast;
}

ctu_t *ctu_expr_string(scan_t *scan, where_t where, char *text, size_t length)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprString);
    ast->text = text;
    ast->length = length;
    return ast;
}

ctu_t *ctu_expr_init(scan_t *scan, where_t where, const vector_t *inits)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprInit);
    ast->inits = inits;
    return ast;
}

ctu_t *ctu_expr_call(scan_t *scan, where_t where, ctu_t *callee, const vector_t *args)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprCall);
    ast->callee = callee;
    ast->args = args;
    return ast;
}

ctu_t *ctu_expr_name(scan_t *scan, where_t where, const vector_t *path)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprName);
    ast->path = path;
    return ast;
}

ctu_t *ctu_expr_cast(scan_t *scan, where_t where, ctu_t *expr, ctu_t *type)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprCast);
    ast->expr = expr;
    ast->cast = type;
    return ast;
}

ctu_t *ctu_expr_ref(scan_t *scan, where_t where, ctu_t *expr)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprRef);
    ast->expr = expr;
    return ast;
}

ctu_t *ctu_expr_deref(scan_t *scan, where_t where, ctu_t *expr)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprDeref);
    ast->expr = expr;
    return ast;
}

ctu_t *ctu_expr_index(scan_t *scan, where_t where, ctu_t *expr, ctu_t *index)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprIndex);
    ast->expr = expr;
    ast->index = index;
    return ast;
}

ctu_t *ctu_expr_field(scan_t *scan, where_t where, ctu_t *expr, char *field)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprField);
    ast->expr = expr;
    ast->field = field;
    return ast;
}

ctu_t *ctu_expr_field_indirect(scan_t *scan, where_t where, ctu_t *expr, char *field)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprFieldIndirect);
    ast->expr = expr;
    ast->field = field;
    return ast;
}

ctu_t *ctu_expr_unary(scan_t *scan, where_t where, unary_t unary, ctu_t *expr)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprUnary);
    ast->unary = unary;
    ast->expr = expr;
    return ast;
}

ctu_t *ctu_expr_binary(scan_t *scan, where_t where, binary_t binary, ctu_t *lhs, ctu_t *rhs)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprBinary);
    ast->binary = binary;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ctu_t *ctu_expr_compare(scan_t *scan, where_t where, compare_t compare, ctu_t *lhs, ctu_t *rhs)
{
    ctu_t *ast = ctu_new(scan, where, eCtuExprCompare);
    ast->compare = compare;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

/* types */

ctu_t *ctu_type_name(scan_t *scan, where_t where, vector_t *path)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypeName);
    ast->type_name = path;
    return ast;
}

ctu_t *ctu_type_pointer(scan_t *scan, where_t where, ctu_t *pointer)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypePointer);
    ast->pointer = pointer;
    return ast;
}

ctu_t *ctu_type_array(scan_t *scan, where_t where, ctu_t *array, ctu_t *length)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypeArray);
    ast->array_type = array;
    ast->array_length = length;
    return ast;
}

ctu_t *ctu_type_function(scan_t *scan, where_t where, const vector_t *params, ctu_t *return_type)
{
    ctu_t *ast = ctu_new(scan, where, eCtuTypeFunction);
    ast->params = params;
    ast->return_type = return_type;
    return ast;
}

/* decls */

ctu_t *ctu_decl_global(scan_t *scan, where_t where, bool exported, bool mut, char *name, ctu_t *type, ctu_t *value)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclGlobal, name, exported);
    ast->mut = mut;
    ast->type = type;
    ast->value = value;
    return ast;
}

ctu_t *ctu_decl_function(scan_t *scan, where_t where, bool exported, char *name, const vector_t *params, char *variadic, ctu_t *return_type, ctu_t *body)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclFunction, name, exported);
    ast->params = params;
    ast->variadic = variadic;
    ast->return_type = return_type;
    ast->body = body;
    return ast;
}

/* type decls */

ctu_t *ctu_decl_typealias(scan_t *scan, where_t where, bool exported, char *name, bool newtype, ctu_t *type)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclTypeAlias, name, exported);
    ast->newtype = newtype;
    ast->type_alias = type;
    return ast;
}

ctu_t *ctu_decl_union(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclUnion, name, exported);
    ast->fields = fields;
    return ast;
}

ctu_t *ctu_decl_struct(scan_t *scan, where_t where, bool exported, char *name, vector_t *fields)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclStruct, name, exported);
    ast->fields = fields;
    return ast;
}

ctu_t *ctu_decl_variant(scan_t *scan, where_t where, bool exported, char *name, ctu_t *underlying, const vector_t *cases)
{
    ctu_t *ast = ctu_decl(scan, where, eCtuDeclVariant, name, exported);
    ast->underlying = underlying;
    ast->cases = cases;
    return ast;
}

ctu_t *ctu_field(scan_t *scan, where_t where, char *name, ctu_t *type)
{
    ctu_t *ast = ctu_new(scan, where, eCtuField);
    ast->name = name;
    ast->field_type = type;
    return ast;
}

ctu_t *ctu_param(scan_t *scan, where_t where, char *name, ctu_t *type)
{
    ctu_t *ast = ctu_new(scan, where, eCtuParam);
    ast->name = name;
    ast->param_type = type;
    return ast;
}

ctu_t *ctu_field_init(scan_t *scan, where_t where, char *name, ctu_t *value)
{
    ctu_t *ast = ctu_new(scan, where, eCtuFieldInit);
    ast->field = name;
    ast->expr = value;
    return ast;
}

ctu_t *ctu_variant_case(scan_t *scan, where_t where, char *name, bool is_default, ctu_t *expr)
{
    ctu_t *ast = ctu_new(scan, where, eCtuVariantCase);
    ast->name = name;
    ast->default_case = is_default;
    ast->case_value = expr;
    return ast;
}

///
/// extras
///

ctu_params_t ctu_params_new(const vector_t *params, char *variadic)
{
    ctu_params_t result = {
        .params = params,
        .variadic = variadic
    };

    return result;
}
