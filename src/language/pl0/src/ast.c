#include "pl0/ast.h"

#include "base/panic.h"
#include "arena/arena.h"
#include "pl0/scan.h"

pl0_t *pl0_new(scan_t *scan, where_t where, pl0_type_t type)
{
    CTASSERT(scan != NULL);

    pl0_scan_t *ctx = pl0_scan_context(scan);

    pl0_t *node = ARENA_MALLOC(sizeof(pl0_t), "pl0", scan, ctx->ast_arena);
    node->node = node_new(scan, where);
    node->type = type;

    ARENA_IDENTIFY(node->node, "node", node, ctx->ast_arena);

    return node;
}

pl0_t *pl0_digit(scan_t *scan, where_t where, mpz_t digit)
{
    pl0_t *node = pl0_new(scan, where, ePl0Digit);
    mpz_init_set(node->digit, digit);
    return node;
}

pl0_t *pl0_ident(scan_t *scan, where_t where, const char *ident)
{
    pl0_t *node = pl0_new(scan, where, ePl0Ident);
    node->ident = ident;
    return node;
}

pl0_t *pl0_binary(scan_t *scan, where_t where, binary_t binary, pl0_t *lhs, pl0_t *rhs)
{
    pl0_t *node = pl0_new(scan, where, ePl0Binary);
    node->binary = binary;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

pl0_t *pl0_compare(scan_t *scan, where_t where, compare_t compare, pl0_t *lhs, pl0_t *rhs)
{
    pl0_t *node = pl0_new(scan, where, ePl0Compare);
    node->compare = compare;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

pl0_t *pl0_unary(scan_t *scan, where_t where, unary_t unary, pl0_t *operand)
{
    pl0_t *node = pl0_new(scan, where, ePl0Unary);
    node->unary = unary;
    node->operand = operand;
    return node;
}

pl0_t *pl0_odd(scan_t *scan, where_t where, pl0_t *operand)
{
    pl0_t *node = pl0_new(scan, where, ePl0Odd);
    node->operand = operand;
    return node;
}

pl0_t *pl0_print(scan_t *scan, where_t where, pl0_t *operand)
{
    pl0_t *node = pl0_new(scan, where, ePl0Print);
    node->print = operand;
    return node;
}

pl0_t *pl0_assign(scan_t *scan, where_t where, const char *dst, pl0_t *src)
{
    pl0_t *node = pl0_new(scan, where, ePl0Assign);
    node->dst = dst;
    node->src = src;
    return node;
}

pl0_t *pl0_call(scan_t *scan, where_t where, const char *procedure)
{
    pl0_t *node = pl0_new(scan, where, ePl0Call);
    node->procedure = procedure;
    return node;
}

pl0_t *pl0_branch(scan_t *scan, where_t where, pl0_t *cond, pl0_t *then)
{
    pl0_t *node = pl0_new(scan, where, ePl0Branch);
    node->cond = cond;
    node->then = then;
    return node;
}

pl0_t *pl0_loop(scan_t *scan, where_t where, pl0_t *cond, pl0_t *body)
{
    pl0_t *node = pl0_new(scan, where, ePl0Loop);
    node->cond = cond;
    node->then = body;
    return node;
}

pl0_t *pl0_stmts(scan_t *scan, where_t where, vector_t *stmts)
{
    pl0_t *node = pl0_new(scan, where, ePl0Stmts);
    node->stmts = stmts;
    return node;
}

pl0_t *pl0_procedure(scan_t *scan, where_t where, const char *name, vector_t *locals, vector_t *body)
{
    pl0_t *node = pl0_new(scan, where, ePl0Procedure);
    node->name = name;
    node->locals = locals;
    node->body = body;
    return node;
}

pl0_t *pl0_value(scan_t *scan, where_t where, const char *name, pl0_t *value)
{
    pl0_t *node = pl0_new(scan, where, ePl0Value);
    node->name = name;
    node->value = value;
    return node;
}

pl0_t *pl0_import(scan_t *scan, where_t where, vector_t *parts)
{
    pl0_t *node = pl0_new(scan, where, ePl0Import);
    node->path = parts;
    return node;
}

pl0_t *pl0_module(scan_t *scan, where_t where, vector_t *mod, vector_t *imports, vector_t *consts, vector_t *globals,
                  vector_t *procs, pl0_t *entry)
{
    pl0_t *node = pl0_new(scan, where, ePl0Module);
    node->mod = mod;
    node->imports = imports;
    node->consts = consts;
    node->globals = globals;
    node->procs = procs;
    node->entry = entry;
    return node;
}
