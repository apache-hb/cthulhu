#include "json_ast.h"
#include "json_scan.h"

#include "arena/arena.h"
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

static json_t *json_ast_new(scan_t *scan, where_t where, json_kind_t kind)
{
    CTASSERT(scan != NULL);

    arena_t *arena = scan_get_arena(scan);
    json_t *ast = ARENA_MALLOC(sizeof(json_t), "json", scan, arena);
    ast->kind = kind;
    ast->node = node_new(scan, where);
    return ast;
}

json_member_t json_member(text_t key, json_t *value)
{
    json_member_t member = {
        .key = key,
        .value = value,
    };

    return member;
}

json_t *json_ast_string(scan_t *scan, where_t where, text_t string)
{
    json_t *ast = json_ast_new(scan, where, eJsonString);
    ast->string = string;
    return ast;
}

json_t *json_ast_integer(scan_t *scan, where_t where, mpz_t integer)
{
    json_t *ast = json_ast_new(scan, where, eJsonInteger);
    mpz_init_set(ast->integer, integer);
    return ast;
}

json_t *json_ast_float(scan_t *scan, where_t where, float real)
{
    json_t *ast = json_ast_new(scan, where, eJsonFloat);
    ast->real = real;
    return ast;
}

json_t *json_ast_boolean(scan_t *scan, where_t where, bool boolean)
{
    json_t *ast = json_ast_new(scan, where, eJsonBoolean);
    ast->boolean = boolean;
    return ast;
}

json_t *json_ast_array(scan_t *scan, where_t where, const vector_t *array)
{
    CTASSERT(array != NULL);

    json_t *ast = json_ast_new(scan, where, eJsonArray);
    ast->array = array;
    return ast;
}

json_t *json_ast_object(scan_t *scan, where_t where, const typevec_t *members)
{
    json_scan_t *context = json_scan_context(scan);
    logger_t *logger = context->reports;
    size_t len = typevec_len(members);
    map_t *result = map_optimal(len, kTypeInfoString, scan_get_arena(scan));

    for (size_t i = 0; i < len; i++)
    {
        json_member_t *member = typevec_offset(members, i);
        text_t key = member->key;
        json_t *prev = map_get(result, key.text);

        if (prev != NULL)
        {
            // TODO: Report error
            (void)logger;
            (void)prev;
        }

        map_set(result, key.text, member->value);
    }

    json_t *ast = json_ast_new(scan, where, eJsonObject);
    ast->object = result;
    return ast;
}

json_t *json_ast_empty_object(scan_t *scan, where_t where)
{
    json_t *ast = json_ast_new(scan, where, eJsonObject);
    ast->object = map_new(1, kTypeInfoString, scan_get_arena(scan));
    return ast;
}

json_t *json_ast_null(scan_t *scan, where_t where)
{
    return json_ast_new(scan, where, eJsonNull);
}
