#pragma once

#include "core/compiler.h"

typedef struct typevec_t typevec_t;
typedef struct json_t json_t;
typedef struct arena_t arena_t;

CT_BEGIN_API

typedef enum ast_kind_t
{
    // const char *
    eAstString,

    // text_t
    eAstText,

    // bool
    eAstBool,

    // vector_t*
    eAstVector,

    // map_t*
    eAstMap,

    // typevec_t*
    eAstTypevec,

    // mpz_t
    eAstMpz,

    eAstCount
} ast_kind_t;

typedef struct ast_field_t
{
    const char *name;
    ast_kind_t kind;
} ast_field_t;

typedef struct ast_node_t
{
    struct ast_node_t *parent;
    const char *name;
    typevec_t *fields;
} ast_node_t;

typedef struct ast_decls_t
{
    typevec_t *decls;
} ast_decls_t;

ast_decls_t meta_build_decls(const json_t *json, arena_t *arena);

CT_END_API
