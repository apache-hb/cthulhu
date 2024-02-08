#include "meta/ast.h"

#include "notify/notify.h"

#include "arena/arena.h"

#include "base/panic.h"
#include "base/util.h"

#include "std/typed/vector.h"
#include "std/vector.h"

#include "json/json.h"

static bool is_of_type(const json_t *json, const char *name, json_kind_t kind, logger_t *logger)
{
    if (json == NULL)
    {
        msg_notify(logger, &kEvent_UnexpectedJsonType, NULL, "%s: expected %s got NULL", name, json_kind_name(kind));
        return false;
    }

    if (json->kind != kind)
    {
        msg_notify(logger, &kEvent_UnexpectedJsonType, json->node, "%s: expected %s got %s", name, json_kind_name(kind), json_kind_name(json->kind));
        return false;
    }

    return true;
}

static ast_kind_t match_kind(text_t text)
{
    if (ctu_strncmp(text.text, "cstring", text.length) == 0) return eAstString;
    if (ctu_strncmp(text.text, "mpz", text.length) == 0) return eAstMpz;
    if (ctu_strncmp(text.text, "bool", text.length) == 0) return eAstBool;
    if (ctu_strncmp(text.text, "node", text.length) == 0) return eAstNode;

    return eAstInvalid;
}

static ast_field_t parse_field(const json_t *json, logger_t *logger, arena_t *arena)
{
    CTASSERT(json != NULL);
    CTASSERT(logger != NULL);

    ast_field_t result = {
        .name = NULL,
        .kind = eAstInvalid,
    };

    if (!is_of_type(json, "field", eJsonObject, logger))
    {
        return result;
    }

    json_t *name_json = json_map_get(json, "name");
    if (!is_of_type(name_json, "name", eJsonString, logger))
    {
        return result;
    }

    text_t name = name_json->string;
    result.name = arena_strndup(name.text, name.length, arena);

    json_t *type_json = json_map_get(json, "type");
    if (!is_of_type(type_json, "type", eJsonString, logger))
    {
        return result;
    }

    text_t type = type_json->string;
    result.kind = match_kind(type);

    return result;
}

static ast_node_t parse_node(const json_t *json, logger_t *logger, arena_t *arena)
{
    CTASSERT(json != NULL);
    CTASSERT(logger != NULL);

    ast_node_t result = {
        .name = NULL,
        .fields = NULL,
    };

    if (!is_of_type(json, "node", eJsonObject, logger))
    {
        return result;
    }

    json_t *name_json = json_map_get(json, "name");
    if (!is_of_type(name_json, "name", eJsonString, logger))
    {
        return result;
    }

    text_t name = name_json->string;
    result.name = arena_strndup(name.text, name.length, arena);

    json_t *fields_json = json_map_get(json, "fields");
    if (!is_of_type(fields_json, "fields", eJsonArray, logger))
    {
        return result;
    }

    size_t len = vector_len(fields_json->array);
    typevec_t *fields = typevec_new(sizeof(ast_field_t), len, arena);

    for (size_t i = 0; i < len; i++)
    {
        json_t *field_json = json_array_get(fields_json, i);
        if (!is_of_type(field_json, "field", eJsonObject, logger))
        {
            continue;
        }

        ast_field_t field = parse_field(field_json, logger, arena);
        if (field.name != NULL)
        {
            typevec_push(fields, &field);
        }
    }

    result.fields = fields;

    return result;
}

ast_decls_t meta_build_decls(const json_t *json, logger_t *logger, arena_t *arena)
{
    CTASSERT(json != NULL);
    CTASSERT(logger != NULL);

    typevec_t *decls = typevec_new(sizeof(ast_node_t), 16, arena);
    ast_decls_t result = {
        .decls = decls,
    };

    if (!is_of_type(json, "root", eJsonObject, logger))
    {
        return result;
    }

    json_t *decls_json = json_map_get(json, "decls");
    if (!is_of_type(decls_json, "decls", eJsonArray, logger))
    {
        return result;
    }

    size_t len = vector_len(decls_json->array);

    for (size_t i = 0; i < len; i++)
    {
        json_t *decl_json = json_array_get(decls_json, i);
        if (!is_of_type(decl_json, "decl", eJsonObject, logger))
        {
            continue;
        }

        ast_node_t decl = parse_node(decl_json, logger, arena);
        if (decl.name != NULL)
        {
            typevec_push(decls, &decl);
        }
    }

    return result;
}
