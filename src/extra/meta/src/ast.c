#include "meta/ast.h"

#include "arena/arena.h"

#include "scan/node.h"
#include "scan/scan.h"

#include "base/panic.h"

meta_field_t meta_field_new(const char *name, meta_ast_t *type)
{
    meta_field_t field = {
        .name = name,
        .type = type
    };

    return field;
}

meta_config_t meta_config_new(const char *name, const char *value)
{
    meta_config_t config = {
        .name = name,
        .value = value
    };

    return config;
}

static meta_ast_t *meta_ast_new(scan_t *scan, where_t where, meta_kind_t kind)
{
    CTASSERT(scan != NULL);

    arena_t *arena = scan_get_arena(scan);
    meta_ast_t *ast = ARENA_MALLOC(sizeof(meta_ast_t), NULL, "meta ast", arena);
    ast->kind = kind;
    ast->node = node_new(scan, where);
    return ast;
}

meta_ast_t *meta_module(scan_t *scan, where_t where, map_t *config, vector_t *nodes)
{
    meta_ast_t *ast = meta_ast_new(scan, where, eMetaModule);
    ast->config = config;
    ast->nodes = nodes;
    return ast;
}

meta_ast_t *meta_node(scan_t *scan, where_t where, const char *name, typevec_t *fields)
{
    meta_ast_t *ast = meta_ast_new(scan, where, eMetaAstNode);
    ast->name = name;
    ast->fields = fields;
    return ast;
}

meta_ast_t *meta_opaque(scan_t *scan, where_t where, const char *opaque)
{
    meta_ast_t *ast = meta_ast_new(scan, where, eMetaOpaque);
    ast->opaque = opaque;
    return ast;
}

meta_ast_t *meta_vector(scan_t *scan, where_t where, const meta_ast_t *element)
{
    meta_ast_t *ast = meta_ast_new(scan, where, eMetaVector);
    ast->element = element;
    return ast;
}

meta_ast_t *meta_string(scan_t *scan, where_t where)
{
    return meta_ast_new(scan, where, eMetaString);
}

meta_ast_t *meta_ast(scan_t *scan, where_t where)
{
    return meta_ast_new(scan, where, eMetaNode);
}

meta_ast_t *meta_digit(scan_t *scan, where_t where)
{
    return meta_ast_new(scan, where, eMetaDigit);
}
