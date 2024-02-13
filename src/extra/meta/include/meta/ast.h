#pragma once

#include "core/compiler.h"
#include "core/where.h"

#include "notify/diagnostic.h" // IWYU pragma: export

typedef struct typevec_t typevec_t;
typedef struct vector_t vector_t;
typedef struct node_t node_t;
typedef struct logger_t logger_t;
typedef struct map_t map_t;
typedef struct arena_t arena_t;
typedef struct scan_t scan_t;

CT_BEGIN_API

#define NEW_EVENT(id, ...) extern const diagnostic_t kEvent_##id;
#include "meta.def"

typedef struct meta_ast_t meta_ast_t;

typedef enum meta_kind_t
{
    eMetaModule,
    eMetaAstNode,

    eMetaVector,
    eMetaString,
    eMetaNode,
    eMetaOpaque,
    eMetaDigit,

    eMetaCount
} meta_kind_t;

typedef struct meta_config_t
{
    const char *name;
    const char *value;
} meta_config_t;

typedef struct meta_field_t
{
    const char *name;
    meta_ast_t *type;
} meta_field_t;

typedef struct meta_ast_t
{
    meta_kind_t kind;
    const node_t *node;

    union {
        /* eMetaModule */
        struct {
            map_t *config;
            vector_t *nodes;
        };

        /* eMetaAstNode */
        struct {
            const char *name;
            typevec_t *fields;
        };

        /* eAstOpaque */
        const char *opaque;

        /* eAstVector */
        const meta_ast_t *element;
    };
} meta_ast_t;

meta_field_t meta_field_new(const char *name, meta_ast_t *type);
meta_config_t meta_config_new(const char *name, const char *value);

meta_ast_t *meta_module(scan_t *scan, where_t where, map_t *config, vector_t *nodes);
meta_ast_t *meta_node(scan_t *scan, where_t where, const char *name, typevec_t *fields);
meta_ast_t *meta_opaque(scan_t *scan, where_t where, const char *opaque);
meta_ast_t *meta_vector(scan_t *scan, where_t where, const meta_ast_t *element);

meta_ast_t *meta_string(scan_t *scan, where_t where);
meta_ast_t *meta_ast(scan_t *scan, where_t where);
meta_ast_t *meta_digit(scan_t *scan, where_t where);

CT_END_API
