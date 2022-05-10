#include "sema.h"
#include "ast.h"

#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"

#include "cthulhu/report/report-ext.h"
#include "cthulhu/util/set.h"
#include "cthulhu/util/str.h"

typedef enum
{
    TAG_VARS,    // hlir_t*
    TAG_PROCS,   // hlir_t*
    TAG_TYPES,   // hlir_t*
    TAG_MODULES, // sema_t*

    TAG_MAX
} tag_t;

typedef struct
{
    size_t totalDecls;
    hlir_t *parentModule;
} sema_data_t;

static bool is_discard_ident(const char *id)
{
    return id == NULL || str_equal(id, "$");
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast);

static hlir_t *sema_typename(sema_t *sema, ast_t *ast)
{
    size_t len = vector_len(ast->path);
    if (len > 1)
    {
        ctu_assert(sema->reports, "typename path can only be 1 element long currently");
        return hlir_error(ast->node, "typename path can only be 1 element long currently");
    }

    const char *name = vector_tail(ast->path);
    hlir_t *decl = sema_get(sema, TAG_TYPES, name);
    if (decl == NULL)
    {
        report(sema->reports, ERROR, ast->node, "type '%s' not found", name);
        return hlir_error(ast->node, "type not found");
    }

    return decl;
}

static hlir_t *sema_pointer(sema_t *sema, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->type);
    return hlir_pointer(ast->node, NULL, type, ast->indexable);
}

static hlir_t *sema_array(sema_t *sema, ast_t *ast)
{
    UNUSED(sema);

    report(sema->reports, ERROR, ast->node, "array not implemented");
    return hlir_error(ast->node, "array not implemented");
}

static hlir_t *sema_closure(sema_t *sema, ast_t *ast)
{
    hlir_t *result = sema_type(sema, ast->result);
    size_t len = vector_len(ast->params);
    vector_t *params = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *param = vector_get(ast->params, i);
        hlir_t *type = sema_type(sema, param);
        vector_set(params, i, type);

        if (hlir_is(type, HLIR_VOID))
        {
            report(sema->reports, ERROR, param->node, "void parameter");
        }
    }

    return hlir_closure(ast->node, NULL, params, result, ast->variadic);
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast)
{
    switch (ast->of)
    {
    case AST_TYPENAME:
        return sema_typename(sema, ast);
    case AST_POINTER:
        return sema_pointer(sema, ast);
    case AST_ARRAY:
        return sema_array(sema, ast);
    case AST_CLOSURE:
        return sema_closure(sema, ast);
    default:
        report(sema->reports, INTERNAL, ast->node, "unknown sema-type: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-type");
    }
}

static void check_duplicates_and_add_fields(sema_t *sema, vector_t *fields, hlir_t *decl)
{
    size_t len = vector_len(fields);
    set_t *names = set_new(len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *field = vector_get(fields, i);
        const char *name = field->name;

        if (!is_discard_ident(name))
        {
            if (set_contains(names, name))
            {
                report(sema->reports, ERROR, field->node, "field '%s' already defined", name);
                continue;
            }

            set_add(names, name);
        }

        if (field->field == NULL)
        {
            continue;
        }

        hlir_t *type = sema_type(sema, field->field);
        hlir_t *entry = hlir_field(field->node, type, name);
        hlir_set_parent(entry, decl);

        hlir_add_field(decl, entry);
    }

    set_delete(names);
}

static void sema_struct(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    vector_t *fields = ast->fields;

    check_duplicates_and_add_fields(sema, fields, decl);

    hlir_build_struct(decl);
}

static void sema_union(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    vector_t *fields = ast->fields;

    check_duplicates_and_add_fields(sema, fields, decl);

    hlir_build_union(decl);
}

static void sema_alias(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->alias);
    hlir_build_alias(decl, type, false);
}

/**
 * variants are internally represented as
 * struct {
 *   tag_type tag;
 *   union {
 *      fields...
 *   }
 * }
 */
static void sema_variant(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    // build the tag
    {
        // create the variant tag
        char *tagName = format("%s_tag", ast->name);
        hlir_t *tag = hlir_digit(ast->node, tagName, DIGIT_INT, SIGN_UNSIGNED);

        // create the field container for the tag
        hlir_t *field = hlir_field(ast->node, tag, "tag");
        hlir_set_parent(field, decl);

        // add the field to the struct
        hlir_add_field(decl, field);
    }

    // build the data
    {
        // create the variant data holder
        char *unionName = format("%s_data", ast->name);
        hlir_t *innerUnion = hlir_begin_union(ast->node, unionName);

        // add all fields with data to the union
        check_duplicates_and_add_fields(sema, ast->fields, innerUnion);

        // then build the union and set its parent
        hlir_build_union(innerUnion);
        hlir_set_parent(innerUnion, decl);

        // create the field container for the union
        hlir_t *dataField = hlir_field(ast->node, innerUnion, "data");
        hlir_set_parent(dataField, decl);

        // add the field to the struct
        hlir_add_field(decl, dataField);
    }

    // add the variant to the struct
    hlir_build_struct(decl);
}

static void sema_decl(sema_t *sema, ast_t *ast)
{
    hlir_t *decl;

    switch (ast->of)
    {
    case AST_STRUCTDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_struct(sema, decl, ast);
        break;

    case AST_UNIONDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_union(sema, decl, ast);
        break;

    case AST_ALIASDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_alias(sema, decl, ast);
        break;

    case AST_VARIANTDECL:
        decl = sema_get(sema, TAG_TYPES, ast->name);
        sema_variant(sema, decl, ast);
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        break;
    }
}

static void add_decl(sema_t *sema, tag_t tag, const char *name, hlir_t *decl)
{
    if (is_discard_ident(name))
    {
        const node_t *node = get_hlir_node(decl);
        report(sema->reports, ERROR, node, "discarding declaration");
        return;
    }

    hlir_t *other = sema_get(sema, tag, name);
    if (other != NULL)
    {
        const node_t *node = get_hlir_node(decl);
        const node_t *otherNode = get_hlir_node(other);
        report_shadow(sema->reports, name, node, otherNode);
        return;
    }

    sema_set(sema, tag, name, decl);
}

static void fwd_decl(sema_t *sema, ast_t *ast)
{
    hlir_t *decl;

    switch (ast->of)
    {
    case AST_STRUCTDECL:
        decl = hlir_begin_struct(ast->node, ast->name);
        break;

    case AST_UNIONDECL:
        decl = hlir_begin_union(ast->node, ast->name);
        break;

    case AST_ALIASDECL:
        decl = hlir_begin_alias(ast->node, ast->name);
        break;

    case AST_VARIANTDECL:
        decl = hlir_begin_struct(ast->node, ast->name);
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        return;
    }

    sema_data_t *semaData = sema_get_data(sema);

    hlir_set_parent(decl, semaData->parentModule);
    add_decl(sema, TAG_TYPES, ast->name, decl);
}

typedef struct
{
    const char *name;
    digit_t width;
    sign_t sign;
} basic_digit_t;

static const basic_digit_t kBasicDigits[] = {
    {"char", DIGIT_CHAR, SIGN_SIGNED},      {"uchar", DIGIT_CHAR, SIGN_UNSIGNED}, {"short", DIGIT_SHORT, SIGN_SIGNED},
    {"ushort", DIGIT_SHORT, SIGN_UNSIGNED}, {"int", DIGIT_INT, SIGN_SIGNED},      {"uint", DIGIT_INT, SIGN_UNSIGNED},
    {"long", DIGIT_LONG, SIGN_SIGNED},      {"ulong", DIGIT_LONG, SIGN_UNSIGNED},
};

#define TOTAL_BASIC_DIGITS (sizeof(kBasicDigits) / sizeof(basic_digit_t))

static void add_basic_types(sema_t *sema)
{
    const node_t *node = node_builtin();

    add_decl(sema, TAG_TYPES, "void", hlir_void(node, "void"));

    add_decl(sema, TAG_TYPES, "bool", hlir_bool(node, "bool"));
    add_decl(sema, TAG_TYPES, "str", hlir_string(node, "str"));

    for (size_t i = 0; i < TOTAL_BASIC_DIGITS; i++)
    {
        const basic_digit_t *basicDigit = &kBasicDigits[i];
        hlir_t *digit = hlir_digit(node, basicDigit->name, basicDigit->width, basicDigit->sign);
        add_decl(sema, TAG_TYPES, basicDigit->name, digit);
    }

    // enable the below later

    // special types for interfacing with C
    // add_decl(sema, TAG_TYPES, "enum", hlir_digit(node, "enum", DIGIT_INT, SIGN_SIGNED));
}

void ctu_forward_decls(runtime_t *runtime, compile_t *compile)
{
    ast_t *root = compile->ast;

    size_t totalDecls = vector_len(root->decls);
    size_t sizes[] = {
        [TAG_VARS] = totalDecls,
        [TAG_PROCS] = totalDecls,
        [TAG_TYPES] = totalDecls,
        [TAG_MODULES] = totalDecls,
    };

    sema_t *sema = sema_new(NULL, runtime->reports, TAG_MAX, sizes);

    char *name = NULL;
    if (root->modspec != NULL)
    {
        name = str_join(".", root->modspec->path);
    }

    hlir_t *mod = hlir_module(root->node, name, vector_of(0), vector_of(0), vector_of(0));

    sema_data_t semaData = {.totalDecls = totalDecls, .parentModule = mod};

    sema_set_data(sema, BOX(semaData));

    add_basic_types(sema);

    for (size_t i = 0; i < totalDecls; i++)
    {
        ast_t *decl = vector_get(root->decls, i);
        fwd_decl(sema, decl);
    }

    vector_t *types = map_values(sema_tag(sema, TAG_TYPES));
    vector_t *globals = map_values(sema_tag(sema, TAG_VARS));
    vector_t *procs = map_values(sema_tag(sema, TAG_PROCS));

    hlir_update_module(mod, types, globals, procs);

    compile->sema = sema;
    compile->hlir = mod;
}

void ctu_process_imports(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);
    UNUSED(compile);
}

void ctu_compile_module(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);

    ast_t *root = compile->ast;
    sema_t *sema = compile->sema;
    sema_data_t *data = sema_get_data(sema);

    for (size_t i = 0; i < data->totalDecls; i++)
    {
        ast_t *decl = vector_get(root->decls, i);
        sema_decl(sema, decl);
    }

    vector_t *types = map_values(sema_tag(sema, TAG_TYPES));
    vector_t *globals = map_values(sema_tag(sema, TAG_VARS));
    vector_t *procs = map_values(sema_tag(sema, TAG_PROCS));

    hlir_update_module(compile->hlir, types, globals, procs);
}
