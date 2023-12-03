#include "pl0/sema.h"
#include "pl0/ast.h"

#include "cthulhu/util/util.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "core/macros.h"
#include "base/util.h"
#include "memory/memory.h"
#include "base/panic.h"

#include "std/str.h"
#include "std/map.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

static const tree_t *gIntType = NULL;
static const tree_t *gCharType = NULL;
static const tree_t *gBoolType = NULL;
static const tree_t *gVoidType = NULL;

static const tree_t *gIntRef = NULL;

static tree_t *gPrint = NULL;
static tree_t *gPrintString = NULL;

static const tree_attribs_t kPrintAttrib = {
    .link = eLinkImport,
    .visibility = eVisiblePublic,
    .mangle = "printf"
};

static const tree_attribs_t kExportAttrib = {
    .link = eLinkExport,
    .visibility = eVisiblePublic
};

static const tree_attribs_t kEntryAttrib = {
    .link = eLinkEntryCli,
    .visibility = eVisiblePrivate
};

static char *pl0_normalize(const char *name)
{
    return str_lower(name);
}

static void report_pl0_shadowing(reports_t *reports, const char *name, const node_t *prevDefinition, const node_t *newDefinition)
{
    message_t *id = report_shadow(reports, name, prevDefinition, newDefinition);
    report_note(id, "PL/0 is case insensitive");
}

static tree_t *get_decl(tree_t *sema, const char *name, const pl0_tag_t *tags, size_t len)
{
    char *id = pl0_normalize(name);
    for (size_t i = 0; i < len; i++)
    {
        pl0_tag_t tag = tags[i];
        tree_t *decl = tree_module_get(sema, tag, id);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}

static tree_t *get_var(tree_t *sema, const char *name)
{
    const pl0_tag_t kTags[] = { ePl0TagValues, ePl0TagImportedValues };

    return get_decl(sema, name, kTags, sizeof(kTags) / sizeof(pl0_tag_t));
}

static tree_t *get_proc(tree_t *sema, const char *name)
{
    const pl0_tag_t kTags[] = { ePl0TagProcs, ePl0TagImportedProcs };

    return get_decl(sema, name, kTags, sizeof(kTags) / sizeof(pl0_tag_t));
}

static void set_decl(tree_t *sema, pl0_tag_t tag, const char *name, tree_t *decl)
{
    char *id = pl0_normalize(name);
    tree_module_set(sema, tag, id, decl);
}

static void set_proc(tree_t *sema, pl0_tag_t tag, const char *name, tree_t *proc)
{
    tree_t *other = get_proc(sema, name);
    if (other != NULL && other != proc)
    {
        const node_t *node = tree_get_node(proc);
        const node_t *otherNode = tree_get_node(other);
        report_pl0_shadowing(sema->reports, name, otherNode, node);
        return;
    }

    set_decl(sema, tag, name, proc);
}

static void set_var(tree_t *sema, pl0_tag_t tag, const char *name, tree_t *tree)
{
    tree_t *other = get_var(sema, name);
    if (other != NULL && other != tree)
    {
        const node_t *node = tree_get_node(tree);
        const node_t *otherNode = tree_get_node(other);

        report_pl0_shadowing(sema->reports, name, otherNode, node);
        return;
    }

    set_decl(sema, tag, name, tree);
}

static tree_t *make_runtime_mod(lifetime_t *lifetime)
{
    size_t decls[ePl0TagTotal] = {
        [ePl0TagValues] = 1,
        [ePl0TagTypes] = 1,
        [ePl0TagProcs] = 1,
        [ePl0TagModules] = 1
    };

    tree_t *mod = lifetime_sema_new(lifetime, "runtime", ePl0TagTotal, decls);
    set_proc(mod, ePl0TagProcs, "print", gPrint);
    set_decl(mod, ePl0TagValues, "$fmt", gPrintString);
    return mod;
}

static vector_t *make_runtime_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "pl0");
    vector_push(&path, "lang");
    return path;
}

static tree_t *get_string_type(size_t size)
{
    node_t *node = node_builtin();
    return tree_type_pointer(node, "string", tree_type_digit(node, "$", eDigitChar, eSignSigned, eQualConst), size);
}

static tree_storage_t get_const_storage(void)
{
    tree_storage_t storage = {
        .storage = gIntType,
        .size = 1,
        .quals = eQualConst
    };

    return storage;
}

static tree_storage_t get_mutable_storage(void)
{
    tree_storage_t storage = {
        .storage = gIntType,
        .size = 1,
        .quals = eQualMutable
    };

    return storage;
}

void pl0_init(driver_t *handle)
{
    node_t *node = node_builtin();
    lifetime_t *lifetime = handle_get_lifetime(handle);

    gIntType = tree_type_digit(node, "integer", eDigitInt, eSignSigned, eQualUnknown);
    gCharType = tree_type_digit(node, "char", eDigitChar, eSignSigned, eQualUnknown);
    gBoolType = tree_type_bool(node, "boolean", eQualConst);
    gVoidType = tree_type_unit(node, "void");

    gIntRef = tree_type_reference(node, "ref", gIntType);

    tree_t *stringType = get_string_type(4);

    vector_t *params = vector_of(1);
    vector_set(params, 0, tree_decl_param(node, "fmt", stringType));

    const tree_storage_t storage = {
        .storage = gCharType,
        .size = 4,
        .quals = eQualConst
    };

    gPrintString = tree_decl_global(node, "$fmt", storage, stringType, tree_expr_string(node, stringType, "%d\n", 4));
    tree_set_attrib(gPrintString, &kExportAttrib);

    tree_t *signature = tree_type_closure(node, "print", gIntType, params, eArityVariable);
    gPrint = tree_decl_function(node, "print", signature, params, vector_of(0), NULL);
    tree_set_attrib(gPrint, &kPrintAttrib);

    tree_t *runtime = make_runtime_mod(lifetime);
    vector_t *path = make_runtime_path();

    context_t *ctx = compiled_new(handle, runtime);
    add_context(handle_get_lifetime(handle), path, ctx);
}

static void report_pl0_unresolved(reports_t *reports, const node_t *node, const char *name)
{
    report(reports, eFatal, node, "unresolved reference to `%s`", name);
}

///
/// sema
///

static tree_t *sema_expr(tree_t *sema, pl0_t *node);
static tree_t *sema_compare(tree_t *sema, pl0_t *node);
static tree_t *sema_stmt(tree_t *sema, pl0_t *node);

static tree_t *sema_digit(pl0_t *node)
{
    return tree_expr_digit(node->node, gIntType, node->digit);
}

static tree_t *sema_ident(tree_t *sema, pl0_t *node)
{
    tree_t *var = get_var(sema, node->ident);
    if (var == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->ident);
        return tree_error(node->node, "unresolved identifier `%s`", node->ident);
    }

    return tree_resolve(tree_get_cookie(sema), var);
}

static tree_t *sema_binary(tree_t *sema, pl0_t *node)
{
    tree_t *lhs = sema_expr(sema, node->lhs);
    tree_t *rhs = sema_expr(sema, node->rhs);
    return tree_expr_binary(node->node, gIntType, node->binary, lhs, rhs);
}

static tree_t *sema_unary(tree_t *sema, pl0_t *node)
{
    tree_t *operand = sema_expr(sema, node->operand);
    return tree_expr_unary(node->node, node->unary, operand);
}

static tree_t *sema_expr(tree_t *sema, pl0_t *node)
{
    switch (node->type)
    {
    case ePl0Digit:
        return sema_digit(node);
    case ePl0Ident:
        return tree_expr_load(node->node, sema_ident(sema, node));
    case ePl0Binary:
        return sema_binary(sema, node);
    case ePl0Unary:
        return sema_unary(sema, node);
    default:
        return tree_raise(node->node, sema->reports, "sema-expr: %d", node->type);
    }
}

static tree_t *sema_vector(tree_t *sema, node_t *node, vector_t *body)
{
    size_t len = vector_len(body);
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        pl0_t *it = vector_get(body, i);
        tree_t *temp = sema_stmt(sema, it);
        vector_set(result, i, temp);
    }

    return tree_stmt_block(node, result);
}

static tree_t *sema_stmts(tree_t *sema, pl0_t *node)
{
    return sema_vector(sema, node->node, node->stmts);
}

static tree_t *sema_call(tree_t *sema, pl0_t *node)
{
    tree_t *proc = get_proc(sema, node->procedure);
    if (proc == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->procedure);
        return tree_error(node->node, "unresolved procedure");
    }

    vector_t *args = vector_new(0);

    return util_create_call(sema, node->node, proc, args);
}

static tree_t *sema_branch(tree_t *sema, pl0_t *node)
{
    tree_t *cond = sema_compare(sema, node->cond);
    tree_t *then = sema_stmt(sema, node->then);

    return tree_stmt_branch(node->node, cond, then, NULL);
}

static tree_t *sema_assign(tree_t *sema, pl0_t *node)
{
    tree_t *dst = get_var(sema, node->dst);
    tree_t *src = sema_expr(sema, node->src);

    if (dst == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->dst);
        return tree_error(node->node, "unresolved variable");
    }

    const tree_t *dstType = tree_get_type(dst);
    quals_t quals = tree_ty_get_quals(dstType);

    if (quals & eQualConst)
    {
        report(sema->reports, eFatal, node->node, "cannot assign to constant value");
    }

    return tree_stmt_assign(node->node, dst, src);
}

static tree_t *sema_loop(tree_t *sema, pl0_t *node)
{
    tree_t *cond = sema_compare(sema, node->cond);
    tree_t *body = sema_stmt(sema, node->then);

    return tree_stmt_loop(node->node, cond, body, NULL);
}

static tree_t *sema_print(tree_t *sema, pl0_t *node)
{
    tree_t *expr = sema_expr(sema, node->print);

    vector_t *args = vector_of(2);
    vector_set(args, 0, gPrintString);
    vector_set(args, 1, expr);

    return tree_expr_call(node->node, gPrint, args);
}

static tree_t *sema_stmt(tree_t *sema, pl0_t *node)
{
    switch (node->type)
    {
    case ePl0Stmts: return sema_stmts(sema, node);
    case ePl0Call: return sema_call(sema, node);
    case ePl0Branch: return sema_branch(sema, node);
    case ePl0Loop: return sema_loop(sema, node);
    case ePl0Assign: return sema_assign(sema, node);
    case ePl0Print: return sema_print(sema, node);
    default: return tree_raise(node->node, sema->reports, "sema-stmt: %d", node->type);
    }
}

static tree_t *sema_global(tree_t *sema, pl0_t *node)
{
    pl0_t *val = node->value;
    if (val == NULL)
    {
        mpz_t zero;
        mpz_init_set_ui(zero, 0);
        return tree_expr_digit(node->node, gIntType, zero);
    }
    else
    {
        return sema_expr(sema, val);
    }
}

static tree_t *sema_odd(tree_t *sema, pl0_t *node)
{
    mpz_t two;
    mpz_init_set_ui(two, 2);

    mpz_t one;
    mpz_init_set_ui(one, 1);

    tree_t *val = sema_expr(sema, node->operand);
    tree_t *twoValue = tree_expr_digit(node->node, gIntType, two);
    tree_t *oneValue = tree_expr_digit(node->node, gIntType, one);
    tree_t *rem = tree_expr_binary(node->node, gIntType, eBinaryRem, val, twoValue);
    tree_t *eq = tree_expr_compare(node->node, gBoolType, eCompareEq, rem, oneValue);

    return eq;
}

static tree_t *sema_comp(tree_t *sema, pl0_t *node)
{
    tree_t *lhs = sema_expr(sema, node->lhs);
    tree_t *rhs = sema_expr(sema, node->rhs);

    return tree_expr_compare(node->node, gBoolType, node->compare, lhs, rhs);
}

static tree_t *sema_compare(tree_t *sema, pl0_t *node)
{
    switch (node->type)
    {
    case ePl0Odd: return sema_odd(sema, node);
    case ePl0Compare: return sema_comp(sema, node);
    default: return tree_raise(node->node, sema->reports, "sema-compare: %d", node->type);
    }
}

static void sema_proc(tree_t *sema, tree_t *tree, pl0_t *node)
{
    size_t len = vector_len(node->locals);
    size_t sizes[ePl0TagTotal] = {[ePl0TagValues] = len};

    tree_t *nest = tree_module(sema, node->node, node->name, ePl0TagTotal, sizes);

    for (size_t i = 0; i < len; i++)
    {
        pl0_t *local = vector_get(node->locals, i);
        tree_t *it = tree_decl_local(local->node, local->name, get_mutable_storage(), gIntRef);
        set_var(nest, ePl0TagValues, local->name, it);
        tree_add_local(tree, it);
    }

    tree_t *ret = tree_stmt_return(node->node, tree_expr_unit(node->node, gVoidType));

    tree_t *inner = sema_vector(nest, node->node, node->body);

    vector_t *body = vector_new(2);
    vector_push(&body, inner);
    vector_push(&body, ret);

    // make sure we have a return statement
    tree_t *stmts = tree_stmt_block(node->node, body);

    tree_close_function(tree, stmts);
}

static void resolve_global(tree_t *sema, tree_t *decl, void *user)
{
    tree_close_global(decl, sema_global(sema, user));
}

static void resolve_proc(tree_t *sema, tree_t *decl, void *user)
{
    sema_proc(sema, decl, user);
}

static void insert_module(tree_t *sema, tree_t *other)
{
    map_iter_t otherValues = map_iter(tree_module_tag(other, ePl0TagValues));
    map_iter_t otherProcs = map_iter(tree_module_tag(other, ePl0TagProcs));

    while (map_has_next(&otherValues))
    {
        tree_t *decl = map_next(&otherValues).value;
        if (!tree_has_vis(decl, eVisiblePublic)) continue;

        set_var(sema, ePl0TagImportedValues, tree_get_name(decl), decl);
    }

    while (map_has_next(&otherProcs))
    {
        tree_t *decl = map_next(&otherProcs).value;
        if (!tree_has_vis(decl, eVisiblePublic)) continue;

        set_proc(sema, ePl0TagImportedProcs, tree_get_name(decl), decl);
    }
}

typedef struct {
    vector_t *consts;
    vector_t *globals;
    vector_t *procs;
} sema_data_t;

void pl0_forward_decls(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);

    pl0_t *root = context_get_ast(context);
    reports_t *reports = lifetime_get_reports(lifetime);
    cookie_t *cookie = lifetime_get_cookie(lifetime);

    size_t totalConsts = vector_len(root->consts);
    size_t totalGlobals = vector_len(root->globals);
    size_t totalFunctions = vector_len(root->procs);

    const char *id = vector_len(root->mod) > 0
        ? vector_tail(root->mod)
        : context_get_name(context);

    size_t sizes[ePl0TagTotal] = {
        [ePl0TagValues] = totalConsts + totalGlobals,
        [ePl0TagProcs] = totalFunctions,
        [ePl0TagImportedValues] = 64,
        [ePl0TagImportedProcs] = 64
    };

    tree_t *sema = tree_module_root(reports, cookie, root->node, id, ePl0TagTotal, sizes);

    // forward declare everything
    for (size_t i = 0; i < totalConsts; i++)
    {
        pl0_t *it = vector_get(root->consts, i);

        tree_resolve_info_t resolve = {
            .sema = sema,
            .user = it,
            .fnResolve = resolve_global
        };

        tree_t *tree = tree_open_global(it->node, it->name, gIntRef, resolve);
        tree_set_storage(tree, get_const_storage());
        tree_set_attrib(tree, &kExportAttrib);

        set_var(sema, ePl0TagValues, it->name, tree);
    }

    for (size_t i = 0; i < totalGlobals; i++)
    {
        pl0_t *it = vector_get(root->globals, i);

        tree_resolve_info_t resolve = {
            .sema = sema,
            .user = it,
            .fnResolve = resolve_global
        };

        tree_t *tree = tree_open_global(it->node, it->name, gIntRef, resolve);
        tree_set_storage(tree, get_mutable_storage());
        tree_set_attrib(tree, &kExportAttrib);

        set_var(sema, ePl0TagValues, it->name, tree);
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        pl0_t *it = vector_get(root->procs, i);

        tree_t *signature = tree_type_closure(it->node, it->name, gVoidType, vector_of(0), eArityFixed);
        tree_resolve_info_t resolve = {
            .sema = sema,
            .user = it,
            .fnResolve = resolve_proc
        };

        tree_t *tree = tree_open_function(it->node, it->name, signature, resolve);
        tree_set_attrib(tree, &kExportAttrib);

        set_proc(sema, ePl0TagProcs, it->name, tree);
    }


    context_update(context, root, sema);
}

void pl0_process_imports(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);
    pl0_t *root = context_get_ast(context);
    tree_t *sema = context_get_module(context);

    size_t totalImports = vector_len(root->imports);
    for (size_t i = 0; i < totalImports; i++)
    {
        pl0_t *importDecl = vector_get(root->imports, i);
        CTASSERT(importDecl->type == ePl0Import);

        context_t *ctx = get_context(lifetime, importDecl->path);

        if (ctx == NULL)
        {
            report(sema->reports, eFatal, importDecl->node, "cannot import `%s`, failed to find module", str_join(".", importDecl->path));
            continue;
        }

        tree_t *lib = context_get_module(ctx);

        if (lib == sema)
        {
            report(sema->reports, eFatal, importDecl->node, "module cannot import itself");
            continue;
        }

        insert_module(sema, lib);
    }
}

void pl0_compile_module(context_t *context)
{
    pl0_t *root = context_get_ast(context);
    tree_t *mod = context_get_module(context);

    if (root->entry != NULL)
    {
        tree_t *body = sema_stmt(mod, root->entry);
        vector_push(&body->stmts, tree_stmt_return(root->node, tree_expr_unit(root->node, gVoidType)));

        // this is the entry point, we only support cli entry points in pl/0 for now
        tree_t *signature = tree_type_closure(root->node, tree_get_name(mod), gVoidType, vector_of(0), eArityFixed);
        tree_t *tree = tree_decl_function(root->node, tree_get_name(mod), signature, vector_of(0), vector_of(0), body);
        tree_set_attrib(tree, &kEntryAttrib);

        set_decl(mod, ePl0TagProcs, tree_get_name(mod), tree); // TODO: this is a hack
    }
}
