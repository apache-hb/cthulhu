#include "sema.h"
#include "ast.h"

#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/type.h"
#include "cthulhu/interface/runtime.h"
#include "base/util.h"

#include "cthulhu/report/report-ext.h"
#include "std/set.h"
#include "std/str.h"

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

static const char *kDigitNames[SIGN_TOTAL][DIGIT_TOTAL] = {
    [SIGN_SIGNED] =
        {
            [DIGIT_CHAR] = "char",
            [DIGIT_SHORT] = "short",
            [DIGIT_INT] = "int",
            [DIGIT_LONG] = "long",

            [DIGIT_PTR] = "intptr",
            [DIGIT_SIZE] = "size",
            [DIGIT_MAX] = "intmax",
        },
    [SIGN_UNSIGNED] =
        {
            [DIGIT_CHAR] = "uchar",
            [DIGIT_SHORT] = "ushort",
            [DIGIT_INT] = "uint",
            [DIGIT_LONG] = "ulong",

            [DIGIT_PTR] = "uintptr",
            [DIGIT_SIZE] = "usize",
            [DIGIT_MAX] = "uintmax",
        },
};

static hlir_t *kVoidType = NULL;
static hlir_t *kBoolType = NULL;
static hlir_t *kStringType = NULL;
static hlir_t *kDigitTypes[SIGN_TOTAL * DIGIT_TOTAL];

#define DIGIT_INDEX(sign, digit) ((sign)*DIGIT_TOTAL + (digit))

static hlir_t *get_digit_type(sign_t sign, digit_t digit)
{
    return kDigitTypes[DIGIT_INDEX(sign, digit)];
}

static const char *get_digit_name(sign_t sign, digit_t digit)
{
    return kDigitNames[sign][digit];
}

static sema_t *kRootSema = NULL;

static hlir_t *get_common_type(node_t node, const hlir_t *lhs, const hlir_t *rhs)
{
    if (hlir_is(lhs, HLIR_DIGIT) && hlir_is(rhs, HLIR_DIGIT))
    {
        // get the largest size
        digit_t width = MAX(lhs->width, rhs->width);
        // if either is signed the result is signed
        sign_t sign = (lhs->sign == SIGN_SIGNED || rhs->sign == SIGN_SIGNED) ? SIGN_SIGNED : SIGN_UNSIGNED;

        return get_digit_type(sign, width);
    }

    return hlir_error(node, "unknown common type");
}

static void add_decl(sema_t *sema, tag_t tag, const char *name, hlir_t *decl);

static void add_basic_types(sema_t *sema)
{
    add_decl(sema, TAG_TYPES, "void", kVoidType);
    add_decl(sema, TAG_TYPES, "bool", kBoolType);
    add_decl(sema, TAG_TYPES, "str", kStringType);

    for (int sign = 0; sign < SIGN_TOTAL; sign++)
    {
        for (int digit = 0; digit < DIGIT_TOTAL; digit++)
        {
            const char *name = get_digit_name(sign, digit);
            hlir_t *type = get_digit_type(sign, digit);

            add_decl(sema, TAG_TYPES, name, type);
        }
    }

    // enable the below later

    // special types for interfacing with C
    // add_decl(sema, TAG_TYPES, "enum", hlir_digit(node, "enum", DIGIT_INT, SIGN_SIGNED));
}

void ctu_init_compiler(runtime_t *runtime)
{
    size_t sizes[TAG_MAX] = {
        [TAG_VARS] = 1,
        [TAG_PROCS] = 1,
        [TAG_TYPES] = 32,
        [TAG_MODULES] = 1,
    };

    kRootSema = sema_new(NULL, runtime->reports, TAG_MAX, sizes);

    node_t node = node_builtin();

    kVoidType = hlir_void(node, "void");
    kBoolType = hlir_bool(node, "bool");
    kStringType = hlir_string(node, "str");

    for (int sign = 0; sign < SIGN_TOTAL; sign++)
    {
        for (int digit = 0; digit < DIGIT_TOTAL; digit++)
        {
            const char *name = get_digit_name(sign, digit);
            kDigitTypes[DIGIT_INDEX(sign, digit)] = hlir_digit(node, name, digit, sign);
        }
    }

    add_basic_types(kRootSema);
}

static bool is_discard_ident(const char *id)
{
    return id == NULL || str_equal(id, "$");
}

static hlir_t *sema_type(sema_t *sema, ast_t *ast);

static hlir_t *sema_expr(sema_t *sema, ast_t *ast);

static hlir_t *sema_typename(sema_t *sema, ast_t *ast)
{
    size_t len = vector_len(ast->path);
    sema_t *current = sema;
    for (size_t i = 0; i < len - 1; i++)
    {
        const char *name = vector_get(ast->path, i);
        sema_t *next = sema_get(current, TAG_MODULES, name);

        if (next == NULL)
        {
            report(sema->reports, ERROR, ast->node, "unknown namespace `%s`", name);
            return hlir_error(ast->node, "unknown namespace");
        }

        current = next;
    }

    const char *name = vector_tail(ast->path);
    while (current != NULL)
    {
        hlir_t *decl = sema_get(current, TAG_TYPES, name);
        if (decl != NULL)
        {
            return decl;
        }

        current = current->parent;
    }

    report(sema->reports, ERROR, ast->node, "type '%s' not found", name);
    return hlir_error(ast->node, "type not found");
}

static hlir_t *sema_pointer(sema_t *sema, ast_t *ast)
{
    hlir_t *type = sema_type(sema, ast->type);
    return hlir_pointer(ast->node, NULL, type, ast->indexable);
}

static hlir_t *sema_array(sema_t *sema, ast_t *ast)
{
    hlir_t *size = sema_expr(sema, ast->size);
    hlir_t *type = sema_type(sema, ast->type);

    return hlir_array(sema->reports, ast->node, NULL, type, size);
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

static hlir_t *sema_digit(ast_t *ast)
{
    // TODO: check if the digit is in range
    // TODO: digit suffixes should be added later
    // TODO: or maybe we want untyped literals?

    const hlir_t *type = get_digit_type(SIGN_SIGNED, DIGIT_INT);
    return hlir_digit_literal(ast->node, type, ast->digit);
}

static hlir_t *sema_bool(ast_t *ast)
{
    return hlir_bool_literal(ast->node, kBoolType, ast->boolean);
}

static hlir_t *sema_binary(sema_t *sema, ast_t *ast)
{
    hlir_t *lhs = sema_expr(sema, ast->lhs);
    hlir_t *rhs = sema_expr(sema, ast->rhs);

    hlir_t *type = get_common_type(ast->node, get_hlir_type(lhs), get_hlir_type(rhs));

    if (!hlir_is(type, HLIR_DIGIT))
    {
        report(sema->reports, ERROR, ast->node, "cannot perform binary operations on %s", get_hlir_name(type));
    }

    return hlir_binary(ast->node, type, ast->binary, lhs, rhs);
}

static hlir_t *sema_expr(sema_t *sema, ast_t *ast)
{
    switch (ast->of)
    {
    case AST_DIGIT:
        return sema_digit(ast);
    case AST_BOOL:
        return sema_bool(ast);
    case AST_BINARY:
        return sema_binary(sema, ast);

    default:
        report(sema->reports, INTERNAL, ast->node, "unknown sema-expr: %d", ast->of);
        return hlir_error(ast->node, "unknown sema-expr");
    }
}

static hlir_t *sema_stmt(sema_t *sema, ast_t *stmt);

static hlir_t *sema_stmts(sema_t *sema, ast_t *stmts)
{
    size_t len = vector_len(stmts->stmts);

    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        ast_t *stmt = vector_get(stmts->stmts, i);
        hlir_t *hlir = sema_stmt(sema, stmt);
        vector_set(result, i, hlir);
    }

    return hlir_stmts(stmts->node, result);
}

static hlir_t *sema_return(sema_t *sema, ast_t *ast)
{
    hlir_t *result = sema_expr(sema, ast->operand);

    return hlir_return(ast->node, result);
}

static hlir_t *sema_stmt(sema_t *sema, ast_t *stmt)
{
    switch (stmt->of)
    {
    case AST_RETURN:
        return sema_return(sema, stmt);

    case AST_STMTS:
        return sema_stmts(sema, stmt);

    default:
        report(sema->reports, INTERNAL, stmt->node, "unknown sema-stmt: %d", stmt->of);
        return hlir_error(stmt->node, "unknown sema-stmt");
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
        hlir_add_field(decl, entry);
    }
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

        // add the field to the struct
        hlir_add_field(decl, dataField);
    }

    // add the variant to the struct
    hlir_build_struct(decl);
}

static void sema_func(sema_t *sema, hlir_t *decl, ast_t *ast)
{
    UNUSED(sema);

    hlir_attributes_t *attribs = hlir_attributes(ast->body == NULL ? LINK_IMPORTED : LINK_EXPORTED, 0, NULL, NULL);
    hlir_t *body = NULL;
    if (ast->body == NULL)
    {
        body = hlir_stmts(ast->node, vector_new(0));
    }
    else
    {
        size_t tags[TAG_MAX] = {[TAG_VARS] = 32, [TAG_PROCS] = 32, [TAG_TYPES] = 32, [TAG_MODULES] = 32};

        sema_t *nest = sema_new(sema, NULL, TAG_MAX, tags);

        body = sema_stmts(nest, ast->body);
    }

    hlir_build_function(decl, body);
    hlir_set_attributes(decl, attribs);
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

    case AST_FUNCDECL:
        decl = sema_get(sema, TAG_PROCS, ast->name);
        sema_func(sema, decl, ast);
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
        node_t node = get_hlir_node(decl);
        report(sema->reports, ERROR, node, "discarding declaration");
        return;
    }

    hlir_t *other = sema_get(sema, tag, name);
    if (other != NULL)
    {
        node_t node = get_hlir_node(decl);
        node_t otherNode = get_hlir_node(other);
        report_shadow(sema->reports, name, node, otherNode);
        return;
    }

    sema_set(sema, tag, name, decl);
}

static hlir_t *begin_function(sema_t *sema, ast_t *ast)
{
    hlir_t *result = kVoidType;
    ast_t *signature = ast->signature;
    if (signature->result != NULL)
    {
        result = sema_type(sema, signature->result);
    }

    signature_t sig = {.params = vector_new(0), .result = result, .variadic = false};

    return hlir_begin_function(ast->node, ast->name, sig);
}

static void fwd_decl(sema_t *sema, ast_t *ast)
{
    hlir_t *decl;
    tag_t tag = TAG_TYPES;

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

    case AST_FUNCDECL:
        decl = begin_function(sema, ast);
        tag = TAG_PROCS;
        break;

    default:
        ctu_assert(sema->reports, "unexpected ast of type %d", ast->of);
        return;
    }

    sema_data_t *semaData = sema_get_data(sema);

    hlir_set_parent(decl, semaData->parentModule);
    add_decl(sema, tag, ast->name, decl);
}

static char *make_import_name(vector_t *vec)
{
    return str_join(".", vec);
}

static void import_namespaced_decls(sema_t *sema, ast_t *import, sema_t *mod)
{
    const char *name = vector_tail(import->path);
    sema_t *previous = sema_get(sema, TAG_MODULES, name);

    if (previous != NULL)
    {
        message_t *id =
            report(sema->reports, ERROR, import->node, "a module was already imported under the name `%s`", name);
        report_note(id, "use module aliases to avoid name collisions");
        return;
    }

    sema_set(sema, TAG_MODULES, name, mod);
}

void ctu_forward_decls(runtime_t *runtime, compile_t *compile)
{
    ast_t *root = compile->ast;

    logverbose("decls: %p", root->decls);

    size_t totalDecls = vector_len(root->decls);
    size_t sizes[TAG_MAX] = {
        [TAG_VARS] = totalDecls,
        [TAG_PROCS] = totalDecls,
        [TAG_TYPES] = totalDecls,
        [TAG_MODULES] = vector_len(root->imports),
    };

    sema_t *sema = sema_new(kRootSema, runtime->reports, TAG_MAX, sizes);

    char *name = NULL;
    if (root->modspec != NULL)
    {
        name = make_import_name(root->modspec->path);
    }

    hlir_t *mod = hlir_module(root->node, name, vector_of(0), vector_of(0), vector_of(0));

    sema_data_t semaData = {.totalDecls = totalDecls, .parentModule = mod};

    sema_set_data(sema, BOX(semaData));

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
    ast_t *root = compile->ast;
    sema_t *sema = compile->sema;

    vector_t *imports = root->imports;
    size_t totalImports = vector_len(imports);

    for (size_t i = 0; i < totalImports; i++)
    {
        ast_t *import = vector_get(imports, i);
        char *name = make_import_name(import->path);

        sema_t *mod = find_module(runtime, name);
        if (mod == NULL)
        {
            report(runtime->reports, ERROR, import->node, "module '%s' not found", name);
            continue;
        }

        if (mod == sema)
        {
            report(runtime->reports, WARNING, import->node, "module cannot import itself");
            continue;
        }

        import_namespaced_decls(sema, import, mod);
    }
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
