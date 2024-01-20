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
    CTASSERT(name != NULL);

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

ref_ast_t *ref_program(scan_t *scan, where_t where, vector_t *mod, vector_t *imports, vector_t *decls)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstProgram);
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

ref_ast_t *ref_field(scan_t *scan, where_t where, char *name, ref_ast_t *type, ref_ast_t *value)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstField, name);
    ast->type = type;
    ast->initial = value;
    return ast;
}

ref_ast_t *ref_method(scan_t *scan, where_t where, bool const_method, char *name, vector_t *params, ref_ast_t *type, ref_ast_t *body)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstMethod, name);
    ast->return_type = type;
    ast->method_params = params;
    ast->body = body;
    ast->const_method = const_method;
    return ast;
}

ref_ast_t *ref_param(scan_t *scan, where_t where, char *name, ref_param_t param, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstParam, name);
    ast->param = param;
    ast->type = type;
    return ast;
}

ref_ast_t *ref_instance(scan_t *scan, where_t where, ref_ast_t *type, vector_t *params)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstInstance);
    ast->generic = type;
    ast->params = params;
    return ast;
}

ref_ast_t *ref_pointer(scan_t *scan, where_t where, ref_ast_t *type)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstPointer);
    ast->ptr = type;
    return ast;
}

ref_ast_t *ref_name(scan_t *scan, where_t where, char *ident)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstName);
    ast->ident = ident;
    return ast;
}

ref_ast_t *ref_variant(scan_t *scan, where_t where, char *name, ref_ast_t *underlying, vector_t *cases)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstVariant, name);
    ast->underlying = underlying;
    ast->cases = cases;
    ast->default_case = NULL;

    size_t len = vector_len(cases);
    for (size_t i = 0; i < len; i++)
    {
        ref_ast_t *decl = vector_get(cases, i);
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

    return ast;
}

ref_ast_t *ref_case(scan_t *scan, where_t where, char *name, mpz_t value, bool is_default)
{
    ref_ast_t *ast = ref_ast_decl(scan, where, eAstCase, name);
    mpz_init_set(ast->value, value);
    if (is_default) ast->flags |= eDeclDefault;
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

ref_ast_t *ref_integer(scan_t *scan, where_t where, mpz_t digit)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstInteger);
    mpz_init_set(ast->digit, digit);
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

void ref_set_export(ref_ast_t *ast, bool exported)
{
    CTASSERT(ast != NULL);

    if (exported)
    {
        ast->flags |= eDeclExported;
    }
    else
    {
        ast->flags &= ~eDeclExported;
    }
}

ref_ast_t *ref_attrib_transient(scan_t *scan, where_t where)
{
    return ref_ast_new(scan, where, eAstAttribTransient);
}

ref_ast_t *ref_attrib_deprecated(scan_t *scan, where_t where, typevec_t *message)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribDeprecated);
    typevec_push(message, "\0");
    ast->text = text_make(typevec_data(message), typevec_len(message) - 1);
    return ast;
}

ref_ast_t *ref_attrib_typeid(scan_t *scan, where_t where, mpz_t id)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribTypeId);
    mpz_init_set(ast->id, id);
    return ast;
}

ref_ast_t *ref_attrib_layout(scan_t *scan, where_t where, ref_layout_t layout)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribLayout);
    ast->layout = layout;
    return ast;
}

ref_ast_t *ref_attrib_alignas(scan_t *scan, where_t where, mpz_t align)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribAlign);
    mpz_init_set(ast->align, align);
    return ast;
}

ref_ast_t *ref_attrib_bitflags(scan_t *scan, where_t where)
{
    return ref_ast_new(scan, where, eAstAttribBitflags);
}

ref_ast_t *ref_attrib_arithmatic(scan_t *scan, where_t where)
{
    return ref_ast_new(scan, where, eAstAttribArithmatic);
}

ref_ast_t *ref_attrib_iterator(scan_t *scan, where_t where)
{
    return ref_ast_new(scan, where, eAstAttribIterator);
}

ref_ast_t *ref_attrib_cxxname(scan_t *scan, where_t where, char *ident)
{
    ref_ast_t *ast = ref_ast_new(scan, where, eAstAttribCxxName);
    ast->ident = ident;
    return ast;
}

ref_ast_t *ref_attrib_remote(scan_t *scan, where_t where)
{
    return ref_ast_new(scan, where, eAstAttribRemote);
}

ref_ast_t *ref_attrib_noreflect(scan_t *scan, where_t where)
{
    return ref_ast_new(scan, where, eAstAttribNoReflect);
}
