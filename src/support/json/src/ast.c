// SPDX-License-Identifier: LGPL-3.0-only

#include "json_ast.h"
#include "json_scan.h"

#include "base/panic.h"
#include "notify/notify.h"
#include "std/map.h"
#include "std/typed/vector.h"

static const char *const kJsonNames[eJsonCount] = {
#define JSON_TYPE(id, str) [id] = (str),
#include "json/json.inc"
};

const char *json_kind_name(json_kind_t kind)
{
    CTASSERTF(kind < eJsonCount, "kind=%d", kind);
    return kJsonNames[kind];
}

static json_t json_ast_new(where_t where, json_kind_t kind)
{
    json_t ast = {
        .kind = kind,
        .where = where,
    };

    return ast;
}

json_member_t json_member(text_view_t key, json_t value)
{
    json_member_t member = {
        .key = key,
        .value = value,
    };

    return member;
}

json_t json_ast_string(where_t where, text_view_t string)
{
    json_t ast = json_ast_new(where, eJsonString);
    ast.string = string;
    return ast;
}

json_t json_ast_integer(where_t where, mpz_t integer)
{
    json_t ast = json_ast_new(where, eJsonInteger);
    mpz_init_set(ast.integer, integer);
    return ast;
}

json_t json_ast_float(where_t where, float real)
{
    json_t ast = json_ast_new(where, eJsonFloat);
    ast.real = real;
    return ast;
}

json_t json_ast_boolean(where_t where, bool boolean)
{
    json_t ast = json_ast_new(where, eJsonBoolean);
    ast.boolean = boolean;
    return ast;
}

json_t json_ast_array(where_t where, typevec_t array)
{
    json_t ast = json_ast_new(where, eJsonArray);
    ast.array = array;
    return ast;
}

json_t json_ast_object(scan_t *scan, where_t where, const typevec_t *members)
{
    json_scan_t *context = json_scan_context(scan);
    logger_t *logger = context->reports;
    size_t len = typevec_len(members);
    map_t *result = map_optimal(len, kTypeInfoText, scan_get_arena(scan));

    for (size_t i = 0; i < len; i++)
    {
        json_member_t *member = typevec_offset(members, i);
        text_view_t *key = &member->key;
        json_t *prev = map_get(result, key);

        if (prev != NULL)
        {
            // TODO: Report error
            (void)logger;
            (void)prev;
        }

        map_set(result, key, &member->value);
    }

    json_t ast = json_ast_new(where, eJsonObject);
    ast.object = result;
    return ast;
}

json_t json_ast_empty_object(where_t where)
{
    json_t ast = json_ast_new(where, eJsonObject);
    ast.object = &kEmptyMap;
    return ast;
}

json_t json_ast_null(where_t where)
{
    return json_ast_new(where, eJsonNull);
}
