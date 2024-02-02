#include "ref/ast.h"
#include "arena/arena.h"
#include "base/panic.h"
#include "base/util.h"
#include "cthulhu/events/events.h"
#include "ref/scan.h"
#include "scan/node.h"
#include "scan/scan.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include <string.h>

static ref_ast_t *ref_ast_new(scan_t *scan, where_t where, ref_kind_t kind)
{
    CTASSERT(scan != NULL);

    arena_t *arena = scan_get_arena(scan);
    node_t *node = node_new(scan, where);

    ref_ast_t *ast = ARENA_MALLOC(sizeof(ref_ast_t), "ref_ast_t", scan, arena);
    ast->kind = kind;
    ast->node = node;

    return ast;
}

static ref_ast_t *ref_ast_decl(scan_t *scan, where_t where, ref_kind_t kind, char *name)
{
    ref_ast_t *ast = ref_ast_new(scan, where, kind);
    ast->flags = eDeclNone;
    ast->name = name;
    ast->privacy = ePrivacyDefault;
    ast->attributes = &kEmptyVector;
    return ast;
}

ref_ast_t *ref_unary(scan_t *scan, where_t where, unary_t op, ref_ast_t *expr)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstUnary);
    ast->unary = op;
    ast->expr = expr;
    return ast;
}

ref_pair_t ref_pair(char *ident, typevec_t *body)
{
    ref_pair_t pair = {
        .ident = ident,
        .body = ref_make_string(body),
    };
    return pair;
}

char *ref_make_string(typevec_t *text)
{
    typevec_push(text, "\0");
    return typevec_data(text);
}

ref_ast_t *ref_binary(scan_t *scan, where_t where, binary_t op, ref_ast_t *lhs, ref_ast_t *rhs)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstBinary);
    ast->binary = op;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ref_ast_t *ref_compare(scan_t *scan, where_t where, compare_t op, ref_ast_t *lhs, ref_ast_t *rhs)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstCompare);
    ast->compare = op;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

ref_ast_t *ref_program(scan_t *scan, where_t where, vector_t *mod, vector_t *leading, vector_t *decls)
{
    size_t len = vector_len(leading);
    arena_t *arena = scan_get_arena(scan);
    vector_t *imports = vector_new(len / 2, arena);
    ref_ast_t *ast = ref_ast_new(scan, where, eAstProgram);

    memset(ast->config, 0, sizeof(ast->config));

    for (size_t i = 0; i < len; i++)
    {
        ref_ast_t *decl = vector_get(leading, i);
        CTASSERTF(decl != NULL, "%s.leading[%zu] = NULL", decl->name, i);
        switch (decl->kind)
        {
        case eAstImport:
            vector_push(&imports, decl);
            break;

        case eAstConfig:
            CTASSERTF(decl->cfg < eRefConfigCount, "invalid config tag %d", decl->cfg);
            CTASSERTF(ast->config[decl->cfg] == NULL, "duplicate config tag %d", decl->cfg);
            CTASSERTF(decl->expr != NULL, "config tag %d has no value", decl->cfg);
            ast->config[decl->cfg] = decl->expr;
            break;

        default: NEVER("invalid leading decl %d", decl->kind);
        }
    }

    ast->mod = mod;
    ast->imports = imports;
    ast->decls = decls;
    return ast;
}

ref_ast_t *ref_import(scan_t *scan, where_t where, text_t text)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstImport);
    arena_t *arena = scan_get_arena(scan);
    ast->ident = arena_strndup(text.text, text.length, arena);
    return ast;
}

static void build_record_data(ref_ast_t *ast, ref_privacy_t privacy, vector_t *decls, arena_t *arena)
{
    size_t len = vector_len(decls);
    vector_t *fields = vector_new(len / 2, arena);
    vector_t *methods = vector_new(len / 2, arena);

    for (size_t i = 0; i < len; i++)
    {
        ref_ast_t *decl = vector_get(decls, i);
        CTASSERTF(decl != NULL, "%s.decl[%zu] = NULL", ast->name, i);

        switch (decl->kind)
        {
        case eAstMethod:
            if (decl->privacy == ePrivacyDefault) decl->privacy = privacy;
            vector_push(&methods, decl);
            break;

        case eAstField:
            if (decl->privacy == ePrivacyDefault) decl->privacy = privacy;
            vector_push(&fields, decl);
            break;

        default:
            break;
        }
    }

    ast->fields = fields;
    ast->methods = methods;
}

ref_ast_t *ref_class(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent, vector_t *body)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstClass, name);
    ast->parent = parent;
    ast->tparams = params;

    arena_t *arena = scan_get_arena(scan);

    build_record_data(ast, ePrivacyPrivate, body, arena);

    return ast;
}

ref_ast_t *ref_union(scan_t *scan, where_t where, char *name, ref_ast_t *key, vector_t *body)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstUnion, name);
    ast->parent = key;
    ast->fields = body;
    return ast;
}

ref_ast_t *ref_union_field(scan_t *scan, where_t where, vector_t *cases, vector_t *fields)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstUnionField);
    ast->cases = cases;
    ast->fields = fields;
    return ast;
}

ref_ast_t *ref_struct(scan_t *scan, where_t where, char *name, vector_t *params, ref_ast_t *parent, vector_t *body)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstStruct, name);
    ast->parent = parent;
    ast->tparams = params;

    arena_t *arena = scan_get_arena(scan);

    build_record_data(ast, ePrivacyPublic, body, arena);

    return ast;
}

ref_ast_t *ref_privacy(scan_t *scan, where_t where, ref_privacy_t privacy)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstPrivacy);
    ast->privacy = privacy;
    return ast;
}

ref_ast_t *ref_using(scan_t *scan, where_t where, char *name, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstAlias, name);
    ast->type = type;
    return ast;
}

ref_ast_t *ref_field(scan_t *scan, where_t where, char *name, ref_ast_t *type, ref_ast_t *value)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstField, name);
    ast->type = type;
    ast->initial = value;
    return ast;
}

ref_ast_t *ref_method(scan_t *scan, where_t where, ref_flags_t flags, char *name, vector_t *params, ref_ast_t *type, ref_ast_t *body)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstMethod, name);
    ast->return_type = type;
    ast->method_params = params;
    ast->body = body;
    ast->flags = flags;
    return ast;
}

ref_ast_t *ref_param(scan_t *scan, where_t where, char *name, ref_param_t param, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstParam, name);
    ast->param = param;
    ast->type = type;
    return ast;
}

ref_ast_t *ref_ctor(scan_t *scan, where_t where, vector_t *params, ref_ast_t *body)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstConstructor, NULL);
    ast->method_params = params;
    ast->body = body;
    return ast;
}

ref_ast_t *ref_span(scan_t *scan, where_t where, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstSpan);
    ast->ptr = type;
    return ast;
}

ref_ast_t *ref_array(scan_t *scan, where_t where, ref_ast_t *type, ref_ast_t *size)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstArray);
    ast->ptr = type;
    ast->size = size;
    return ast;
}

ref_ast_t *ref_vector(scan_t *scan, where_t where, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstVector);
    ast->ptr = type;
    return ast;
}

ref_ast_t *ref_const(scan_t *scan, where_t where, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstConst);
    ast->type = type;
    return ast;
}

ref_ast_t *ref_pointer(scan_t *scan, where_t where, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstPointer);
    ast->ptr = type;
    return ast;
}

ref_ast_t *ref_reference(scan_t *scan, where_t where, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstReference);
    ast->ptr = type;
    return ast;
}

ref_ast_t *ref_variant(scan_t *scan, where_t where, char *name, ref_ast_t *underlying, vector_t *cases)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstVariant, name);
    ast->parent = underlying;
    ast->methods = &kEmptyVector;
    ast->default_case = NULL;
    size_t len = vector_len(cases);
    vector_t *case_list = vector_new(len, scan_get_arena(scan));
    vector_t *method_list = vector_new(len, scan_get_arena(scan));

    ref_privacy_t privacy = ePrivacyPublic;
    for (size_t i = 0; i < len; i++)
    {
        ref_ast_t *decl = vector_get(cases, i);
        if (decl->kind == eAstMethod)
        {
            decl->privacy = privacy;
            vector_push(&method_list, decl);
            continue;
        }

        CTASSERTF(decl->kind == eAstCase, "variant '%s' contains non-case/method decl", name);

        vector_push(&case_list, decl);

        if (!decl->is_default) continue;

        if (ast->default_case != NULL)
        {
            ref_scan_t *ctx = ref_scan_context(scan);
            const node_t *node = node_new(scan, where);
            msg_notify(ctx->reports, &kEvent_DuplicateAttribute, node, "multiple default cases in variant '%s'", name);
        }
        else
        {
            ast->default_case = decl;
        }
    }

    ast->cases = case_list;
    ast->methods = method_list;

    return ast;
}

ref_ast_t *ref_case(scan_t *scan, where_t where, char *name, ref_ast_t *value, bool is_default)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstCase, name);
    ast->value = value;
    ast->is_default = is_default;
    return ast;
}

ref_ast_t *ref_opaque(scan_t *scan, where_t where, char *ident)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstOpaque);
    ast->ident = ident;
    return ast;
}

ref_ast_t *ref_opaque_text(scan_t *scan, where_t where, text_t text)
{
    arena_t *arena = scan_get_arena(scan);
    char *ident = arena_strndup(text.text, text.length, arena);
    return ref_opaque(scan, where, ident);
}

ref_ast_t *ref_ident(scan_t *scan, where_t where, char *ident)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstIdent);
    ast->ident = ident;
    return ast;
}

ref_ast_t *ref_integer(scan_t *scan, where_t where, mpz_t integer)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstInteger);
    mpz_init_set(ast->integer, integer);
    return ast;
}

ref_ast_t *ref_string(scan_t *scan, where_t where, typevec_t *text)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstString);
    typevec_push(text, "\0");
    ast->text = text_make(typevec_data(text), typevec_len(text) - 1);
    return ast;
}

ref_ast_t *ref_bool(scan_t *scan, where_t where, bool value)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstBool);
    ast->boolean = value;
    return ast;
}

void ref_set_attribs(ref_ast_t *ast, vector_t *attributes)
{
    CTASSERT(ast != NULL);
    CTASSERT(attributes != NULL);

    ast->attributes = attributes;
}

void ref_set_flags(ref_ast_t *ast, ref_flags_t flags)
{
    CTASSERT(ast != NULL);
    ast->flags = flags;

    if (flags & eDeclPrivate)
        ast->privacy = ePrivacyPrivate;

    if (flags & eDeclPublic)
        ast->privacy = ePrivacyPublic;

    if (flags & eDeclProtected)
        ast->privacy = ePrivacyProtected;
}

ref_ast_t *ref_attrib_deprecated(scan_t *scan, where_t where, char *message)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribDeprecated);
    ast->ident = message;
    return ast;
}

ref_ast_t *ref_attrib_typeid(scan_t *scan, where_t where, ref_ast_t *expr)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribTypeId);
    ast->expr = expr;
    return ast;
}

ref_ast_t *ref_attrib_alignas(scan_t *scan, where_t where, ref_ast_t *expr)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribAlign);
    ast->expr = expr;
    return ast;
}

ref_ast_t *ref_attrib_docs(scan_t *scan, where_t where, map_t *docs)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribDocs);
    ast->docs = docs;
    return ast;
}

ref_ast_t *ref_attrib_remote(scan_t *scan, where_t where)
{
    return ref_ast_new(scan, where, eAstAttribRemote);
}

ref_ast_t *ref_attrib_tag(scan_t *scan, where_t where, ref_attrib_tag_t tag)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribTag);
    ast->attrib = tag;
    return ast;
}

ref_ast_t *ref_attrib_string(scan_t *scan, where_t where, ref_attrib_tag_t tag, char *text)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribString);
    ast->attrib = tag;
    ast->ident = text;
    return ast;
}

ref_ast_t *ref_config_tag(scan_t *scan, where_t where, ref_config_tag_t tag, ref_ast_t *expr)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstConfig);
    ast->cfg = tag;
    ast->expr = expr;
    return ast;
}
