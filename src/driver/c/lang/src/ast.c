#include "c/ast.h"
#include "base/panic.h"
#include "memory/arena.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include <string.h>

static c_ast_t *c_ast_new(scan_t *scan, where_t where, c_kind_t kind)
{
    CTASSERT(scan != NULL);
    arena_t *arena = scan_get_arena(scan);
    node_t *node = node_new(scan, where);

    c_ast_t *ast = ARENA_MALLOC(arena, sizeof(c_ast_t), "c_ast_t", scan);
    ast->node = node;
    ast->kind = kind;
    return ast;
}

// module extensions

c_ast_t *c_ast_module_private_fragment(scan_t *scan, where_t where)
{
    return c_ast_new(scan, where, eAstModulePrivateFragment);
}

c_ast_t *c_ast_module_public_fragment(scan_t *scan, where_t where, vector_t *module_path)
{
    CTASSERT(module_path != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstModulePublicFragment);
    ast->module_path = module_path;
    return ast;
}

c_ast_t *c_ast_module_import(scan_t *scan, where_t where, vector_t *module_path)
{
    CTASSERT(module_path != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstModuleImport);
    ast->module_path = module_path;
    return ast;
}

c_ast_t *c_ast_declarator_list(scan_t *scan, where_t where, vector_t *specifiers, vector_t *declarations)
{
    CTASSERT(specifiers != NULL);
    CTASSERT(declarations != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstDeclaratorList);
    ast->specifiers = specifiers;
    ast->declarations = declarations;
    return ast;
}

c_ast_t *c_ast_init_declarator(scan_t *scan, where_t where, c_ast_t *declarator, c_ast_t *initializer)
{
    CTASSERT(declarator != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstInitDeclarator);
    ast->declarator = declarator;
    ast->initializer = initializer;
    return ast;
}

c_ast_t *c_ast_storage_class(scan_t *scan, where_t where, c_storage_class_t storage_class)
{
    c_ast_t *ast = c_ast_new(scan, where, eAstStorageClass);
    ast->storage_class = storage_class;
    return ast;
}

c_ast_t *c_ast_type_qualifier(scan_t *scan, where_t where, c_type_qualifier_t qualifier)
{
    c_ast_t *ast = c_ast_new(scan, where, eAstTypeQualifier);
    ast->type_qualifier = qualifier;
    return ast;
}

c_ast_t *c_ast_type_specifier(scan_t *scan, where_t where, c_type_specifier_t specifier)
{
    c_ast_t *ast = c_ast_new(scan, where, eAstTypeSpecifier);
    ast->type_specifier = specifier;
    return ast;
}

c_ast_t *c_ast_typedef_name(scan_t *scan, where_t where, char *name)
{
    CTASSERT(name != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstTypedefName);
    ast->typedef_name = name;
    return ast;
}

c_ast_t *c_ast_alignas(scan_t *scan, where_t where, c_ast_t *body)
{
    CTASSERT(body != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstAlignas);
    ast->alignas_body = body;
    return ast;
}

c_ast_t *c_ast_type(scan_t *scan, where_t where, vector_t *parts)
{
    CTASSERT(parts != NULL);

    // TODO: merge parts and report errors

    c_ast_t *ast = c_ast_new(scan, where, eAstType);
    ast->typename_parts = parts;
    return ast;
}

// expressions

c_ast_t *c_ast_expand_exprs(scan_t *scan, where_t where, vector_t *exprs)
{
    CTASSERT(exprs != NULL);
    if (vector_len(exprs) == 1)
    {
        return vector_get(exprs, 0);
    }

    c_ast_t *ast = c_ast_new(scan, where, eAstExprList);
    ast->exprs = exprs;
    return ast;
}

c_ast_t *c_ast_cast(scan_t *scan, where_t where, c_ast_t *expr, c_ast_t *type)
{
    CTASSERT(expr != NULL);
    CTASSERT(type != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstCast);
    ast->operand = expr;
    ast->cast_type = type;
    return ast;
}

c_ast_t *c_ast_string(scan_t *scan, where_t where, text_t text)
{
    c_ast_t *ast = c_ast_new(scan, where, eAstString);
    ast->string = typevec_of_array(sizeof(char), text.text, text.size, scan_get_arena(scan));
    return ast;
}

c_ast_t *c_ast_append_string(scan_t *scan, where_t where, c_ast_t *string, text_t text)
{
    CTASSERT(string != NULL);
    CTASSERT(string->kind == eAstString);

    CTASSERTF(scan == node_get_scan(string->node), "string was created in a different scan (new = %s, old = %s)", scan_path(scan), scan_path(node_get_scan(string->node)));

    where_t old = node_get_location(string->node);

    // merge the ranges
    where_t it = {
        .first_line = old.first_line,
        .first_column = old.first_column,
        .last_line = where.last_line,
        .last_column = where.last_column,
    };

    typevec_append(string->string, text.text, text.size);
    string->node = node_new(scan, it);

    return string;
}


// statements

c_ast_t *c_ast_goto(scan_t *scan, where_t where, char *label)
{
    CTASSERT(label != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstGoto);
    ast->label = label;
    return ast;
}

c_ast_t *c_ast_continue(scan_t *scan, where_t where)
{
    return c_ast_new(scan, where, eAstContinue);
}

c_ast_t *c_ast_break(scan_t *scan, where_t where)
{
    return c_ast_new(scan, where, eAstBreak);
}

c_ast_t *c_ast_return(scan_t *scan, where_t where, c_ast_t *value)
{
    c_ast_t *ast = c_ast_new(scan, where, eAstReturn);
    ast->result = value;
    return ast;
}

// attributes

c_ast_t *c_ast_attribute_noreturn(scan_t *scan, where_t where)
{
    return c_ast_new(scan, where, eAstAttributeNoreturn);
}

c_ast_t *c_ast_attribute_callconv(scan_t *scan, where_t where, c_callconv_t callconv)
{
    c_ast_t *ast = c_ast_new(scan, where, eAstAttributeCallConv);
    ast->callconv = callconv;
    return ast;
}

// labels

c_ast_t *c_ast_label(scan_t *scan, where_t where, const char *name)
{
    CTASSERT(name != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstLabel);
    ast->label = name;
    return ast;
}

c_ast_t *c_ast_case(scan_t *scan, where_t where, c_ast_t *value)
{
    CTASSERT(value != NULL);

    c_ast_t *ast = c_ast_new(scan, where, eAstCase);
    ast->case_value = value;
    return ast;
}

c_ast_t *c_ast_default(scan_t *scan, where_t where)
{
    return c_ast_new(scan, where, eAstDefault);
}


void c_ast_apply_export(c_ast_t *ast, bool exported)
{
    CTASSERT(ast != NULL);

    ast->exported = exported;
}

void c_ast_apply_attributes(c_ast_t *ast, vector_t *attributes)
{
    CTASSERT(ast != NULL);
    CTASSERT(attributes != NULL);

    ast->attributes = attributes;
}
