#include "oberon/ast.h"

#include "report/report.h"

#include "std/str.h"

#include "memory/arena.h"
#include "base/panic.h"

static void ensure_block_names_match(scan_t *scan, const node_t *node, const char *type, const char *name, const char *end)
{
    CTASSERTF(type != NULL && name != NULL, "(type=%s, name=%s)", type, name);
    reports_t *reports = scan_reports(scan);

    if (end == NULL) { return; }

    if (!str_equal(name, end))
    {
        message_t *id = report(reports, eWarn, node, "mismatching %s block BEGIN and END names", type);
        report_note(id, "BEGIN name `%s` does not match END name `%s`", name, end);
    }
}

static obr_t *obr_new(scan_t *scan, where_t where, obr_kind_t kind)
{
    CTASSERT(scan != NULL);

    alloc_t *alloc = scan_alloc(scan);
    obr_t *self = arena_malloc(alloc, sizeof(obr_t), "obr", scan);
    self->kind = kind;
    self->node = node_new(scan, where);
    return self;
}

static obr_t *obr_decl(scan_t *scan, where_t where, obr_kind_t kind, char *name, obr_visibility_t vis)
{
    obr_t *self = obr_new(scan, where, kind);
    self->name = name;
    self->visibility = vis;
    return self;
}

static obr_t *obr_decl_from_symbol(scan_t *scan, where_t where, obr_kind_t kind, const obr_symbol_t *symbol)
{
    return obr_decl(scan, where, kind, symbol->name, symbol->visibility);
}

static obr_t *obr_decl_symbol_location(const obr_symbol_t *symbol, obr_kind_t kind)
{
    return obr_decl(symbol->scan, symbol->where, kind, symbol->name, symbol->visibility);
}

obr_t *obr_module(
    scan_t *scan, where_t where, char *name, char *end,
    vector_t *imports, vector_t *decls, vector_t *init
)
{
    obr_t *self = obr_decl(scan, where, eObrModule, name, eObrVisPublic);
    self->imports = imports;
    self->decls = decls;
    self->init = init;

    ensure_block_names_match(scan, self->node, "MODULE", name, end);

    return self;
}

obr_t *obr_import(scan_t *scan, where_t where, char *name, char *symbol)
{
    obr_t *self = obr_decl(scan, where, eObrImport, name, eObrVisPrivate);
    self->symbol = symbol;
    return self;
}

/* decls */

obr_t *obr_decl_type(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *type)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrDeclType, symbol);
    self->type = type;
    return self;
}

obr_t *obr_decl_var(obr_symbol_t *symbol, obr_t *type)
{
    obr_t *self = obr_decl_symbol_location(symbol, eObrDeclVar);
    self->type = type;
    return self;
}

obr_t *obr_decl_const(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *value)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrDeclConst, symbol);
    self->value = value;
    return self;
}

obr_t *obr_decl_procedure(
    scan_t *scan, where_t where, obr_symbol_t *symbol,
    obr_t *receiver, vector_t *params, obr_t *result,
    vector_t *locals, vector_t *body, char *end
)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrDeclProcedure, symbol);

    // only check if this is a procedure, not a forward decl
    if (body != NULL)
    {
        ensure_block_names_match(scan, self->node, "PROCEDURE", symbol->name, end);
    }

    self->receiver = receiver;
    self->params = params;
    self->result = result;
    self->locals = locals;
    self->body = body;
    return self;
}

/* exprs */

obr_t *obr_expr_name(scan_t *scan, where_t where, char *name)
{
    obr_t *self = obr_new(scan, where, eObrExprName);
    self->object = name;
    return self;
}

obr_t *obr_expr_field(scan_t *scan, where_t where, obr_t *expr, char *field)
{
    obr_t *self = obr_new(scan, where, eObrExprField);
    self->expr = expr;
    self->field = field;
    return self;
}

obr_t *obr_expr_cast(scan_t *scan, where_t where, obr_t *expr, obr_t *cast)
{
    obr_t *self = obr_new(scan, where, eObrExprCast);
    self->expr = expr;
    self->cast = cast;
    return self;
}

obr_t *obr_expr_call(scan_t *scan, where_t where, obr_t *expr, vector_t *args)
{
    obr_t *self = obr_new(scan, where, eObrExprCall);
    self->expr = expr;
    self->args = args;
    return self;
}

obr_t *obr_expr_is(scan_t *scan, where_t where, obr_t *lhs, obr_t *rhs)
{
    obr_t *self = obr_new(scan, where, eObrExprIs);
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

obr_t *obr_expr_in(scan_t *scan, where_t where, obr_t *lhs, obr_t *rhs)
{
    obr_t *self = obr_new(scan, where, eObrExprIn);
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

obr_t *obr_expr_compare(scan_t *scan, where_t where, compare_t op, obr_t *lhs, obr_t *rhs)
{
    obr_t *self = obr_new(scan, where, eObrExprCompare);
    self->compare = op;
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

obr_t *obr_expr_binary(scan_t *scan, where_t where, binary_t op, obr_t *lhs, obr_t *rhs)
{
    obr_t *self = obr_new(scan, where, eObrExprBinary);
    self->binary = op;
    self->lhs = lhs;
    self->rhs = rhs;
    return self;
}

obr_t *obr_expr_unary(scan_t *scan, where_t where, unary_t op, obr_t *expr)
{
    obr_t *self = obr_new(scan, where, eObrExprUnary);
    self->unary = op;
    self->expr = expr;
    return self;
}

obr_t *obr_expr_digit(scan_t *scan, where_t where, const mpz_t digit)
{
    obr_t *self = obr_new(scan, where, eObrExprDigit);
    mpz_init_set(self->digit, digit);
    return self;
}

obr_t *obr_expr_string(scan_t *scan, where_t where, char *text, size_t length)
{
    obr_t *self = obr_new(scan, where, eObrExprString);
    self->text = text;
    self->length = length;
    return self;
}

/* stmts */

obr_t *obr_stmt_return(scan_t *scan, where_t where, obr_t *expr)
{
    obr_t *self = obr_new(scan, where, eObrStmtReturn);
    self->expr = expr;
    return self;
}

obr_t *obr_stmt_while(scan_t *scan, where_t where, obr_t *cond, vector_t *then)
{
    obr_t *self = obr_new(scan, where, eObrStmtWhile);
    self->cond = cond;
    self->then = then;
    return self;
}

obr_t *obr_stmt_assign(scan_t *scan, where_t where, obr_t *dst, obr_t *src)
{
    obr_t *self = obr_new(scan, where, eObrStmtAssign);
    self->dst = dst;
    self->src = src;
    return self;
}

/* types */

obr_t *obr_type_name(scan_t *scan, where_t where, char *name)
{
    obr_t *self = obr_new(scan, where, eObrTypeName);
    self->name = name;
    return self;
}

obr_t *obr_type_qual(scan_t *scan, where_t where, char *name, char *symbol)
{
    obr_t *self = obr_decl(scan, where, eObrTypeQual, name, eObrVisPrivate); // TODO: should types need this data?
    self->symbol = symbol;
    return self;
}

obr_t *obr_type_pointer(scan_t *scan, where_t where, obr_t *type)
{
    obr_t *self = obr_new(scan, where, eObrTypePointer);
    self->pointer = type;
    return self;
}

obr_t *obr_type_array(scan_t *scan, where_t where, obr_t *type)
{
    obr_t *self = obr_new(scan, where, eObrTypeArray);
    self->array = type;
    return self;
}

obr_t *obr_type_record(scan_t *scan, where_t where, vector_t *fields)
{
    obr_t *self = obr_new(scan, where, eObrTypeRecord);
    self->fields = fields;
    return self;
}

/* extra */

obr_t *obr_field(obr_symbol_t *symbol, obr_t *type)
{
    obr_t *self = obr_decl_symbol_location(symbol, eObrField);
    self->type = type;
    return self;
}

obr_t *obr_param(obr_symbol_t *symbol, obr_t *type, bool mut)
{
    obr_t *self = obr_decl_symbol_location(symbol, eObrParam);
    self->mut = mut;
    self->type = type;
    return self;
}

obr_t *obr_receiver(scan_t *scan, where_t where, bool mut, char *name, char *type)
{
    obr_t *self = obr_decl(scan, where, eObrReceiver, name, eObrVisPrivate);
    self->mut = mut;
    self->type = obr_type_name(scan, where, type);
    return self;
}

/* symbols */

obr_symbol_t *obr_symbol(scan_t *scan, where_t where, char *name, obr_visibility_t visibility)
{
    alloc_t *alloc = scan_alloc(scan);
    obr_symbol_t *self = arena_malloc(alloc, sizeof(obr_symbol_t), "obr_symbol", scan);
    self->scan = scan;
    self->where = where;
    self->name = name;
    self->visibility = visibility;
    return self;
}

#define EXPAND_INNER(fn, ...) \
    do {\
        size_t len = vector_len(symbols); \
        vector_t *result = vector_of(len); \
        for (size_t i = 0; i < len; i++) \
        {   \
            obr_symbol_t *symbol = vector_get(symbols, i); \
            obr_t *decl = fn(symbol, __VA_ARGS__); \
            vector_set(result, i, decl); \
        } \
        return result; \
    } while (0)

vector_t *obr_expand_vars(vector_t *symbols, obr_t *type)
{
    EXPAND_INNER(obr_decl_var, type);
}

vector_t *obr_expand_fields(vector_t *symbols, obr_t *type)
{
    EXPAND_INNER(obr_field, type);
}

vector_t *obr_expand_params(vector_t *symbols, obr_t *type, bool mut)
{
    EXPAND_INNER(obr_param, type, mut);
}
