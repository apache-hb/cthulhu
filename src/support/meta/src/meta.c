// SPDX-License-Identifier: LGPL-3.0-only
#include "meta.h"
#include "arena/arena.h"
#include "cthulhu/events/events.h"
#include "std/str.h"

#include "json/query.h"

typedef struct json_context_t
{
    logger_t *logger;
    arena_t *arena;
    scan_t *scan;
} json_context_t;

static json_t *inner_query_type(json_context_t *context, json_t *json, const char *expr, json_kind_t kind)
{
    return json_query_type(json, expr, kind, context->logger, context->arena);
}

static meta_type_t get_meta_type(text_view_t id)
{
    if (text_view_equal(id, (text_view_t)CT_TEXT_VIEW("mpz")))
    {
        return eMetaMpz;
    }

    if (text_view_equal(id, (text_view_t)CT_TEXT_VIEW("ast")))
    {
        return eMetaAst;
    }

    if (text_view_equal(id, (text_view_t)CT_TEXT_VIEW("string")))
    {
        return eMetaString;
    }

    return eMetaUnknown;
}

static bool parse_ast_field(json_context_t *ctx, json_t *json, typevec_t *fields)
{
    json_t *name = inner_query_type(ctx, json, "$.name", eJsonString);
    if (name == NULL)
    {
        return false;
    }

    json_t *type = json_map_get(json, "type");
    if (type == NULL)
    {
        msg_notify(ctx->logger, &kEvent_SymbolNotFound, node_new(ctx->scan, json->where), "no field type");
        return false;
    }

    meta_field_t field = {
        .name = name->string,
        .type = get_meta_type(type->string),
    };

    typevec_push(fields, &field);

    return true;
}

static bool parse_ast_node(json_context_t *ctx, json_t *json, typevec_t *nodes)
{
    json_t *name = inner_query_type(ctx, json, "$.name", eJsonString);
    if (name == NULL)
    {
        return false;
    }

    json_t *fields = inner_query_type(ctx, json, "$.fields", eJsonArray);
    if (fields == NULL)
    {
        return false;
    }

    meta_ast_t ast = {
        .name = name->string,
        .fields = typevec_new(sizeof(meta_field_t), typevec_len(&fields->array), ctx->arena),
    };

    for (size_t i = 0; i < typevec_len(&fields->array); i++)
    {
        json_t *field = json_array_get(fields, i);
        if (!parse_ast_field(ctx, field, ast.fields))
        {
            return false;
        }
    }

    typevec_push(nodes, &ast);

    return true;
}

meta_info_t *meta_info_parse(json_t *json, scan_t *scan, logger_t *logger, arena_t *arena)
{
    json_context_t ctx = {
        .logger = logger,
        .arena = arena,
        .scan = scan,
    };

    json_t *prefix = json_query_type(json, "$.prefix", eJsonString, logger, arena);
    if (prefix == NULL)
    {
        return NULL;
    }

    json_t *nodes = json_query_type(json, "$.nodes", eJsonArray, logger, arena);
    if (nodes == NULL)
    {
        return NULL;
    }

    meta_info_t *info = ARENA_MALLOC(sizeof(meta_info_t), "meta_info_parse", NULL, arena);
    info->prefix = prefix->string;

    size_t len = typevec_len(&nodes->array);
    info->nodes = typevec_new(sizeof(meta_ast_t), len, arena);

    for (size_t i = 0; i < len; i++)
    {
        json_t *node = json_array_get(nodes, i);
        if (!parse_ast_node(&ctx, node, info->nodes))
        {
            return NULL;
        }
    }

    return info;
}
