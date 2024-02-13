#include "meta/meta.h"

#include "arena/arena.h"
#include "base/panic.h"
#include "io/io.h"
#include "meta/ast.h"
#include "std/map.h"
#include "std/str.h"
#include "std/typed/vector.h"
#include "std/vector.h"

typedef struct emit_t
{
    arena_t *arena;
    const char *prefix;
    const char *ast_name;

    io_t *header;
    io_t *source;
} emit_t;

static const char *emit_type(emit_t *emit, meta_ast_t *ast, const char *name)
{
    CTASSERTF(ast != NULL, "ast is NULL");
    switch (ast->kind)
    {
    case eMetaDigit:
        return str_format(emit->arena, "mpz_t %s", name);
    case eMetaString:
        return str_format(emit->arena, "const char *%s", name);
    case eMetaOpaque:
        return str_format(emit->arena, "%s %s", ast->opaque, name);
    case eMetaNode:
        return str_format(emit->arena, "%s *%s", emit->ast_name, name);
    case eMetaVector:
        return str_format(emit->arena, "vector_t *%s", name);

    default:
        NEVER("unknown ast type %d", ast->kind);
    }
}

static const char *emit_assign(emit_t *emit, meta_ast_t *ast, const char *dst, const char *src)
{
    CTASSERT(emit != NULL);

    switch (ast->kind)
    {
    case eMetaDigit:
        return str_format(emit->arena, "mpz_init_set(%s, %s)", dst, src);
    case eMetaString:
    case eMetaOpaque:
    case eMetaVector:
    case eMetaNode:
        return str_format(emit->arena, "%s = %s", dst, src);
    default:
        NEVER("unknown ast type %d", ast->kind);
    }
}

static const char *hash_fn(emit_t *emit, const meta_ast_t *ast)
{
    switch (ast->kind)
    {
    case eMetaNode:
        return str_format(emit->arena, "%s_ast_hash", emit->prefix);
    case eMetaString:
        return "str_hash";

    default:
        NEVER("unknown ast type %d", ast->kind);
    }
}

static const char *emit_hash(emit_t *emit, meta_ast_t *ast, const char *expr)
{
    CTASSERT(emit != NULL);

    switch (ast->kind)
    {
    case eMetaDigit:
        return str_format(emit->arena, "hash = hash_combine(hash, mpz_get_ui(%s));", expr);
    case eMetaString:
        return str_format(emit->arena, "hash = hash_combine(hash, str_hash(%s));", expr);
    case eMetaOpaque:
        // assume the opaque type is convertible to an integer
        return str_format(emit->arena, "hash = hash_combine(hash, %s);", expr);
    case eMetaVector:
        return str_format(emit->arena, "hash = hash_combine_vector(hash, (size_t(*)(const void*))%s, %s);", hash_fn(emit, ast->element), expr);
    case eMetaNode:
        return str_format(emit->arena, "hash = hash_combine(hash, %s_ast_hash(%s));", emit->prefix, expr);
    default:
        NEVER("unknown ast type %d", ast->kind);
    }
}

static bool should_include_gmp(meta_ast_t *ast)
{
    CTASSERTF(ast != NULL, "ast is NULL");
    CTASSERTF(ast->kind == eMetaModule, "ast is not a module");
    vector_t *nodes = ast->nodes;

    size_t len = vector_len(nodes);
    for (size_t i = 0; i < len; i++)
    {
        meta_ast_t *node = vector_get(nodes, i);
        CTASSERTF(node->kind == eMetaAstNode, "node is not an ast decl");

        size_t field_len = typevec_len(node->fields);
        for (size_t j = 0; j < field_len; j++)
        {
            meta_field_t *field = typevec_offset(node->fields, j);
            if (field->type->kind == eMetaDigit)
            {
                return true;
            }
        }
    }

    return false;
}

static void emit_enum(emit_t *emit, vector_t *nodes)
{
    CTASSERTF(emit != NULL, "emit is NULL");
    CTASSERTF(nodes != NULL, "nodes is NULL");

    char *prefix = arena_strdup(emit->prefix, emit->arena);
    prefix[0] = str_toupper(prefix[0]);

    io_printf(emit->header, "typedef enum %s_kind_t {\n", emit->prefix);
    size_t len = vector_len(nodes);
    for (size_t i = 0; i < len; i++)
    {
        meta_ast_t *node = vector_get(nodes, i);
        CTASSERTF(node->kind == eMetaAstNode, "node is not an ast decl");

        char *name = arena_strdup(node->name, emit->arena);
        name[0] = str_toupper(name[0]);

        io_printf(emit->header, "\te%s%s,\n", prefix, name);
    }

    io_printf(emit->header, "\te%sCount\n", prefix);
    io_printf(emit->header, "} %s_kind_t;\n", emit->prefix);
}

static void emit_nodes(emit_t *emit, vector_t *nodes, const char *dup)
{
    CTASSERTF(emit != NULL, "emit is NULL");
    CTASSERTF(nodes != NULL, "nodes is NULL");
    CTASSERTF(dup != NULL, "dup is NULL");

    size_t len = vector_len(nodes);
    for (size_t i = 0; i < len; i++)
    {
        meta_ast_t *node = vector_get(nodes, i);

        io_printf(emit->header, "typedef struct %s_%s_t {\n", emit->prefix, node->name);
        size_t field_len = typevec_len(node->fields);
        for (size_t j = 0; j < field_len; j++)
        {
            meta_field_t *field = typevec_offset(node->fields, j);
            char *name = arena_strdup(field->name, emit->arena);
            io_printf(emit->header, "\t%s;\n", emit_type(emit, field->type, name));
        }
        io_printf(emit->header, "} %s_%s_t;\n", emit->prefix, node->name);
    }
}

static void emit_constructors(emit_t *emit, vector_t *nodes)
{
    CTASSERTF(emit != NULL, "emit is NULL");
    CTASSERTF(nodes != NULL, "nodes is NULL");

    size_t len = vector_len(nodes);
    for (size_t i = 0; i < len; i++)
    {
        meta_ast_t *node = vector_get(nodes, i);
        io_printf(emit->header, "%s_ast_t *%s_%s(scan_t *scan, where_t where", emit->prefix, emit->prefix, node->name);
        size_t field_len = typevec_len(node->fields);
        for (size_t j = 0; j < field_len; j++)
        {
            io_printf(emit->header, ", ");
            meta_field_t *field = typevec_offset(node->fields, j);
            char *field_name = arena_strdup(field->name, emit->arena);
            io_printf(emit->header, "%s", emit_type(emit, field->type, field_name));
        }
        io_printf(emit->header, ");\n");
    }
}

static void emit_source_prelude(emit_t *emit)
{
    CTASSERTF(emit != NULL, "emit is NULL");

    io_printf(emit->source, "#include \"arena/arena.h\"\n");
    io_printf(emit->source, "#include \"scan/node.h\"\n");
    io_printf(emit->source, "#include \"scan/scan.h\"\n");
    io_printf(emit->source, "#include \"base/util.h\"\n");
    io_printf(emit->source, "#include \"base/panic.h\"\n");
    io_printf(emit->source, "#include \"std/vector.h\"\n");

    io_printf(emit->source, "\nstatic %s_ast_t *%s_ast_new(scan_t *scan, where_t where, %s_kind_t kind)\n", emit->prefix, emit->prefix, emit->prefix);
    io_printf(emit->source, "{\n");
    io_printf(emit->source, "\tarena_t *arena = scan_get_arena(scan);\n");
    io_printf(emit->source, "\t%s_ast_t *ast = ARENA_MALLOC(sizeof(%s_ast_t), NULL, \"%s ast\", arena);\n", emit->prefix, emit->prefix, emit->prefix);
    io_printf(emit->source, "\tast->kind = kind;\n");
    io_printf(emit->source, "\tast->node = node_new(scan, where);\n");
    io_printf(emit->source, "\treturn ast;\n");
    io_printf(emit->source, "}\n");
}

static void emit_source_constructors(emit_t *emit, vector_t *nodes)
{
    CTASSERTF(emit != NULL, "emit is NULL");
    CTASSERTF(nodes != NULL, "nodes is NULL");

    size_t len = vector_len(nodes);
    for (size_t i = 0; i < len; i++)
    {
        meta_ast_t *node = vector_get(nodes, i);
        io_printf(emit->source, "%s_ast_t *%s_%s(scan_t *scan, where_t where", emit->prefix, emit->prefix, node->name);
        size_t field_len = typevec_len(node->fields);
        for (size_t j = 0; j < field_len; j++)
        {
            io_printf(emit->source, ", ");
            meta_field_t *field = typevec_offset(node->fields, j);
            io_printf(emit->source, "%s", emit_type(emit, field->type, field->name));
        }
        io_printf(emit->source, ")\n");
        io_printf(emit->source, "{\n");
        io_printf(emit->source, "\t%s_%s_t option;\n", emit->prefix, node->name);

        for (size_t j = 0; j < field_len; j++)
        {
            meta_field_t *field = typevec_offset(node->fields, j);
            io_printf(emit->source, "\t%s;\n", emit_assign(emit, field->type, str_format(emit->arena, "option.%s", field->name), field->name));
        }

        char *dup = arena_strdup(emit->prefix, emit->arena);
        dup[0] = str_toupper(dup[0]);

        char *name = arena_strdup(node->name, emit->arena);
        name[0] = str_toupper(name[0]);

        io_printf(emit->source, "\t%s_ast_t *ast = %s_ast_new(scan, where, e%s%s);\n", emit->prefix, emit->prefix, dup, name);
        io_printf(emit->source, "\tast->%s = option;\n", node->name);
        io_printf(emit->source, "\treturn ast;\n");
        io_printf(emit->source, "}\n");
    }
}

static void emit_node_hash(emit_t *emit, vector_t *nodes)
{
    CTASSERTF(emit != NULL, "emit is NULL");
    CTASSERTF(nodes != NULL, "nodes is NULL");

    io_printf(emit->source, "static size_t hash_combine(size_t hash, size_t value)\n");
    io_printf(emit->source, "{\n");
    io_printf(emit->source, "\thash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);\n");
    io_printf(emit->source, "\treturn hash;\n");
    io_printf(emit->source, "}\n");

    io_printf(emit->source, "static size_t hash_combine_vector(size_t hash, size_t (*fn)(const void *), const vector_t *vec)\n");
    io_printf(emit->source, "{\n");
    io_printf(emit->source, "\tfor (size_t i = 0; i < vector_len(vec); i++)\n");
    io_printf(emit->source, "\t{\n");
    io_printf(emit->source, "\t\thash = hash_combine(hash, fn(vector_get(vec, i)));\n");
    io_printf(emit->source, "\t}\n");
    io_printf(emit->source, "\treturn hash;\n");
    io_printf(emit->source, "}\n");

    io_printf(emit->header, "size_t %s_ast_hash(const %s_ast_t *ast);\n", emit->prefix, emit->prefix);

    io_printf(emit->source, "size_t %s_ast_hash(const %s_ast_t *ast)\n", emit->prefix, emit->prefix);
    io_printf(emit->source, "{\n");
    io_printf(emit->source, "\tif (ast == NULL) return 0;\n");
    io_printf(emit->source, "\tsize_t hash = 0;\n");
    io_printf(emit->source, "\thash ^= ast->kind;\n");
    io_printf(emit->source, "\n");
    io_printf(emit->source, "\tswitch (ast->kind)\n");
    io_printf(emit->source, "\t{\n");

    char *dup = arena_strdup(emit->prefix, emit->arena);
    dup[0] = str_toupper(dup[0]);

    size_t len = vector_len(nodes);
    for (size_t i = 0; i < len; i++)
    {
        meta_ast_t *node = vector_get(nodes, i);
        char *name = arena_strdup(node->name, emit->arena);
        name[0] = str_toupper(name[0]);
        io_printf(emit->source, "\tcase e%s%s:\n", dup, name);

        size_t field_len = typevec_len(node->fields);
        for (size_t j = 0; j < field_len; j++)
        {
            meta_field_t *field = typevec_offset(node->fields, j);
            char *field_name = str_format(emit->arena, "ast->%s.%s", node->name, field->name);
            io_printf(emit->source, "\t\t%s\n", emit_hash(emit, field->type, field_name));
        }
        io_printf(emit->source, "\t\tbreak;\n");
    }
    io_printf(emit->source, "\tdefault: NEVER(\"unknown ast type %%d\", ast->kind);\n");
    io_printf(emit->source, "\t}\n");
    io_printf(emit->source, "\n");
    io_printf(emit->source, "\treturn hash;\n");
    io_printf(emit->source, "}\n");
}

void meta_emit(meta_ast_t *ast, io_t *header, io_t *source, arena_t *arena)
{
    CTASSERTF(ast != NULL, "ast is NULL");
    CTASSERTF(header != NULL, "header is NULL");
    CTASSERTF(source != NULL, "source is NULL");
    CTASSERTF(arena != NULL, "arena is NULL");

    CTASSERTF(ast->kind == eMetaModule, "ast is not a module");
    vector_t *nodes = ast->nodes;

    char *header_name = str_filename(io_name(header), arena);
    io_printf(header, "#pragma once\n");
    io_printf(header, "/* autogenerated source, do not edit */\n");
    io_printf(header, "\n");

    io_printf(source, "#include \"%s\"\n", header_name);
    io_printf(source, "/* autogenerated source, do not edit */\n");
    io_printf(source, "\n");

    io_printf(header, "#include \"core/where.h\"\n");

    if (should_include_gmp(ast))
    {
        io_printf(header, "#include <gmp.h>\n");
    }

    const char *include = map_get(ast->config, "include");
    if (include != NULL)
    {
        if (include[0] == '<')
        {
            io_printf(header, "#include %s\n", include);
        }
        else
        {
            io_printf(header, "#include \"%s\"\n", include);
        }
    }

    io_printf(header, "typedef struct vector_t vector_t;\n");
    io_printf(header, "typedef struct node_t node_t;\n");
    io_printf(header, "typedef struct scan_t scan_t;\n");

    const char *prefix = map_get(ast->config, "prefix");
    CTASSERTF(prefix != NULL, "prefix not provided in config");

    emit_t emit = {
        .arena = arena,
        .prefix = prefix,
        .ast_name = str_format(arena, "%s_ast_t", prefix),

        .header = header,
        .source = source,
    };

    // capitalize the first letter of the prefix
    char *dup = arena_strdup(prefix, arena);
    dup[0] = str_toupper(dup[0]);

    io_printf(header, "typedef struct %s_ast_t %s_ast_t;\n", prefix, prefix);

    emit_enum(&emit, nodes);
    emit_nodes(&emit, nodes, dup);

    io_printf(header, "struct %s_ast_t {\n", prefix);
    io_printf(header, "\t%s_kind_t kind;\n", prefix);
    io_printf(header, "\tconst node_t *node;\n");
    io_printf(header, "\n");
    io_printf(header, "\tunion {\n");
    size_t len = vector_len(nodes);
    for (size_t i = 0; i < len; i++)
    {
        meta_ast_t *node = vector_get(nodes, i);
        char *name = arena_strdup(node->name, arena);
        name[0] = str_toupper(name[0]);
        io_printf(header, "\t\t/* e%s%s */\n", dup, name);
        io_printf(header, "\t\t%s_%s_t %s;\n", prefix, node->name, node->name);
    }
    io_printf(header, "\t};\n");
    io_printf(header, "};\n");

    emit_constructors(&emit, nodes);

    emit_source_prelude(&emit);

    emit_source_constructors(&emit, nodes);

    emit_node_hash(&emit, nodes);
}
