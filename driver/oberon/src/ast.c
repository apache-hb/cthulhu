#include "oberon/ast.h"
#include "oberon/scan.h"

#include "report/report.h"

#include "std/str.h"

#include "base/memory.h"
#include "base/panic.h"

static void ensure_block_names_match(scan_t *scan, const node_t *node, const char *type, const char *name, const char *end)
{
    obr_scan_t *data = scan_get(scan);

    if (end == NULL) { return; }

    if (!str_equal(name, end))
    {
        message_t *id = report(data->reports, eWarn, node, "mismatching %s block BEGIN and END names", type);
        report_note(id, "BEGIN name `%s` does not match END name `%s`", name, end);
    }
}

static obr_t *obr_new(scan_t *scan, where_t where, obr_kind_t kind)
{
    CTASSERT(scan != NULL);

    obr_t *self = ctu_malloc(sizeof(obr_t));
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

obr_t *obr_module(scan_t *scan, where_t where, char *name, char *end, vector_t *imports, vector_t *decls)
{
    obr_t *self = obr_decl(scan, where, eObrModule, name, eObrVisPublic);
    self->imports = imports;
    self->decls = decls;

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

obr_t *obr_decl_var(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *type)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrDeclVar, symbol);
    self->type = type;
    return self;
}

obr_t *obr_decl_const(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *value)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrDeclConst, symbol);
    self->value = value;
    return self;
}

obr_t *obr_decl_procedure(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *receiver, vector_t *params)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrDeclProcedure, symbol);
    self->receiver = receiver;
    self->params = params;
    return self;
}

/* exprs */

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

obr_t *obr_field(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *type)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrField, symbol);
    self->type = type;
    return self;
}

obr_t *obr_param(scan_t *scan, where_t where, obr_symbol_t *symbol, obr_t *type, bool mut)
{
    obr_t *self = obr_decl_from_symbol(scan, where, eObrParam, symbol);
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
    obr_symbol_t *self = ctu_malloc(sizeof(obr_symbol_t));
    self->scan = scan;
    self->where = where;
    self->name = name;
    self->visibility = visibility;
    return self;
}

vector_t *obr_expand_vars(vector_t *symbols, obr_t *type)
{
    size_t len = vector_len(symbols);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_symbol_t *symbol = vector_get(symbols, i);
        obr_t *decl = obr_decl_var(symbol->scan, symbol->where, symbol, type);
        vector_set(result, i, decl);
    }
    return result;
}

vector_t *obr_expand_fields(vector_t *symbols, obr_t *type)
{
    size_t len = vector_len(symbols);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_symbol_t *symbol = vector_get(symbols, i);
        obr_t *field = obr_field(symbol->scan, symbol->where, symbol, type);
        vector_set(result, i, field);
    }
    return result;
}

vector_t *obr_expand_params(vector_t *symbols, obr_t *type, bool mut)
{
    size_t len = vector_len(symbols);
    vector_t *result = vector_of(len);
    for (size_t i = 0; i < len; i++)
    {
        obr_symbol_t *symbol = vector_get(symbols, i);
        obr_t *field = obr_param(symbol->scan, symbol->where, symbol, type, mut);
        vector_set(result, i, field);
    }
    return result;
}
