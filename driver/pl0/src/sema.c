#include "pl0/sema.h"
#include "pl0/ast.h"

#include "report/report.h"
#include "report/report-ext.h"

#include "base/macros.h"
#include "base/util.h"
#include "base/memory.h"
#include "base/panic.h"

#include "std/str.h"
#include "std/map.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"

static const h2_t *kStringType = NULL;
static const h2_t *kConstType = NULL;
static const h2_t *kIntType = NULL;
static const h2_t *kBoolType = NULL;
static const h2_t *kVoidType = NULL;

static h2_t *kPrint = NULL;

static const h2_attrib_t kPrintAttrib = {
    .link = eLinkImport,
    .visibility = eVisiblePublic,
    .mangle = "printf"
};

static const h2_attrib_t kExportAttrib = {
    .link = eLinkExport,
    .visibility = eVisiblePublic
};

static const h2_attrib_t kEntryAttrib = {
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

static h2_t *get_decl(h2_t *sema, const char *name, const pl0_tag_t *tags, size_t len)
{
    char *id = pl0_normalize(name);
    for (size_t i = 0; i < len; i++)
    {
        pl0_tag_t tag = tags[i];
        h2_t *decl = h2_module_get(sema, tag, id);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}

static h2_t *get_var(h2_t *sema, const char *name)
{
    const pl0_tag_t kTags[] = { ePl0TagValues, ePl0TagImportedValues };

    return get_decl(sema, name, kTags, sizeof(kTags) / sizeof(pl0_tag_t));
}

static h2_t *get_proc(h2_t *sema, const char *name)
{
    const pl0_tag_t kTags[] = { ePl0TagProcs, ePl0TagImportedProcs };

    return get_decl(sema, name, kTags, sizeof(kTags) / sizeof(pl0_tag_t));
}

static void set_decl(h2_t *sema, pl0_tag_t tag, const char *name, h2_t *decl)
{
    char *id = pl0_normalize(name);
    h2_module_set(sema, tag, id, decl);
}

static void set_proc(h2_t *sema, pl0_tag_t tag, const char *name, h2_t *proc)
{
    h2_t *other = get_proc(sema, name);
    if (other != NULL && other != proc)
    {
        const node_t *node = h2_get_node(proc);
        const node_t *otherNode = h2_get_node(other);
        report_pl0_shadowing(sema->reports, name, otherNode, node);
        return;
    }

    set_decl(sema, tag, name, proc);
}

static void set_var(h2_t *sema, pl0_tag_t tag, const char *name, h2_t *hlir)
{
    h2_t *other = get_var(sema, name);
    if (other != NULL && other != hlir)
    {
        const node_t *node = h2_get_node(hlir);
        const node_t *otherNode = h2_get_node(other);

        report_pl0_shadowing(sema->reports, name, otherNode, node);
        return;
    }

    set_decl(sema, tag, name, hlir);
}

static h2_t *make_runtime_mod(lifetime_t *lifetime)
{
    size_t decls[ePl0TagTotal] = {
        [ePl0TagValues] = 1,
        [ePl0TagTypes] = 1,
        [ePl0TagProcs] = 1,
        [ePl0TagModules] = 1
    };

    h2_t *mod = lifetime_sema_new(lifetime, "runtime", ePl0TagTotal, decls);
    set_proc(mod, ePl0TagProcs, "print", kPrint);
    return mod;
}

static vector_t *make_runtime_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "pl0");
    vector_push(&path, "lang");
    return path;
}

void pl0_init(driver_t *handle)
{
    node_t *node = node_builtin();
    lifetime_t *lifetime = handle_get_lifetime(handle);

    kConstType = h2_type_digit(node, "integer", eDigitInt, eSignSigned);
    kIntType = h2_qualify(node, kConstType, eQualMutable);
    kBoolType = h2_type_bool(node, "boolean");
    kStringType = h2_type_string(node, "string");
    kVoidType = h2_type_unit(node, "void");

    vector_t *params = vector_of(1);
    vector_set(params, 0, h2_decl_param(node, "fmt", kStringType));

    h2_t *signature = h2_type_closure(node, "print", kVoidType, params, eArityVariable);
    kPrint = h2_decl_function(node, "print", signature, vector_of(0), NULL);
    h2_set_attrib(kPrint, &kPrintAttrib);

    h2_t *runtime = make_runtime_mod(lifetime);
    vector_t *path = make_runtime_path();

    context_t *ctx = compiled_new(handle, runtime);
    add_context(handle_get_lifetime(handle), path, ctx);
}

static void report_pl0_unresolved(reports_t *reports, const node_t *node, const char *name)
{
    report(reports, eFatal, node, "unresolved reference to `%s`", name);
}

static h2_t *sema_expr(h2_t *sema, pl0_t *node);
static h2_t *sema_compare(h2_t *sema, pl0_t *node);
static h2_t *sema_stmt(h2_t *sema, pl0_t *node);

static h2_t *sema_digit(pl0_t *node)
{
    return h2_expr_digit(node->node, kConstType, node->digit);
}

static h2_t *sema_ident(h2_t *sema, pl0_t *node)
{
    h2_t *var = get_var(sema, node->ident);
    if (var == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->ident);
        return h2_error(node->node, "unresolved identifier");
    }

    h2_cookie_t *cookie = h2_get_cookie(sema);
    return h2_expr_load(node->node, h2_resolve(cookie, var));
}

static h2_t *sema_binary(h2_t *sema, pl0_t *node)
{
    h2_t *lhs = sema_expr(sema, node->lhs);
    h2_t *rhs = sema_expr(sema, node->rhs);
    return h2_expr_binary(node->node, kConstType, node->binary, lhs, rhs);
}

static h2_t *sema_unary(h2_t *sema, pl0_t *node)
{
    h2_t *operand = sema_expr(sema, node->operand);
    return h2_expr_unary(node->node, node->unary, operand);
}

static h2_t *sema_expr(h2_t *sema, pl0_t *node)
{
    switch (node->type)
    {
    case ePl0Digit:
        return sema_digit(node);
    case ePl0Ident:
        return sema_ident(sema, node);
    case ePl0Binary:
        return sema_binary(sema, node);
    case ePl0Unary:
        return sema_unary(sema, node);
    default:
        report(sema->reports, eInternal, node->node, "sema-expr: %d", node->type);
        return h2_error(node->node, "sema-expr");
    }
}

static h2_t *sema_vector(h2_t *sema, node_t *node, vector_t *body)
{
    size_t len = vector_len(body);
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        pl0_t *it = vector_get(body, i);
        h2_t *temp = sema_stmt(sema, it);
        vector_set(result, i, temp);
    }

    return h2_stmt_block(node, result);
}

static h2_t *sema_stmts(h2_t *sema, pl0_t *node)
{
    return sema_vector(sema, node->node, node->stmts);
}

static h2_t *sema_call(h2_t *sema, pl0_t *node)
{
    h2_t *proc = get_proc(sema, node->procedure);
    if (proc == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->procedure);
        return h2_error(node->node, "unresolved procedure");
    }

    vector_t *args = vector_new(0);

    return h2_expr_call(node->node, proc, args);
}

static h2_t *sema_branch(h2_t *sema, pl0_t *node)
{
    h2_t *cond = sema_compare(sema, node->cond);
    h2_t *then = sema_stmt(sema, node->then);

    return h2_stmt_branch(node->node, cond, then, NULL);
}

static h2_t *sema_assign(h2_t *sema, pl0_t *node)
{
    h2_t *dst = get_var(sema, node->dst);
    h2_t *src = sema_expr(sema, node->src);

    if (dst == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->dst);
        return h2_error(node->node, "unresolved variable");
    }

    const h2_t *dstType = h2_get_type(dst);

    if (!h2_has_quals(dstType, eQualMutable))
    {
        report(sema->reports, eFatal, node->node, "cannot assign to constant value");
    }

    return h2_stmt_assign(node->node, dst, src);
}

static h2_t *sema_loop(h2_t *sema, pl0_t *node)
{
    h2_t *cond = sema_compare(sema, node->cond);
    h2_t *body = sema_stmt(sema, node->then);

    return h2_stmt_loop(node->node, cond, body, NULL);
}

static h2_t *sema_print(h2_t *sema, pl0_t *node)
{
    h2_t *expr = sema_expr(sema, node->print);

    h2_t *fmt = h2_expr_string(node->node, kStringType, "%d\n", 2);

    vector_t *args = vector_of(2);
    vector_set(args, 0, fmt);
    vector_set(args, 1, expr);

    return h2_expr_call(node->node, kPrint, args);
}

static h2_t *sema_stmt(h2_t *sema, pl0_t *node)
{
    switch (node->type)
    {
    case ePl0Stmts:
        return sema_stmts(sema, node);
    case ePl0Call:
        return sema_call(sema, node);
    case ePl0Branch:
        return sema_branch(sema, node);
    case ePl0Loop:
        return sema_loop(sema, node);
    case ePl0Assign:
        return sema_assign(sema, node);
    case ePl0Print:
        return sema_print(sema, node);
    default:
        report(sema->reports, eInternal, node->node, "sema-stmt: %d", node->type);
        return h2_error(node->node, "sema-stmt");
    }
}

static h2_t *sema_global(h2_t *sema, pl0_t *node)
{
    pl0_t *val = node->value;
    if (val == NULL)
    {
        mpz_t zero;
        mpz_init_set_ui(zero, 0);
        return h2_expr_digit(node->node, kConstType, zero);
    }
    else
    {
        return sema_expr(sema, val);
    }
}

static h2_t *sema_odd(h2_t *sema, pl0_t *node)
{
    mpz_t two;
    mpz_init_set_ui(two, 2);

    mpz_t one;
    mpz_init_set_ui(one, 1);

    h2_t *val = sema_expr(sema, node->operand);
    h2_t *twoValue = h2_expr_digit(node->node, kConstType, two);
    h2_t *oneValue = h2_expr_digit(node->node, kConstType, one);
    h2_t *rem = h2_expr_binary(node->node, kConstType, eBinaryRem, val, twoValue);
    h2_t *eq = h2_expr_compare(node->node, kBoolType, eCompareEq, rem, oneValue);

    return eq;
}

static h2_t *sema_comp(h2_t *sema, pl0_t *node)
{
    h2_t *lhs = sema_expr(sema, node->lhs);
    h2_t *rhs = sema_expr(sema, node->rhs);

    return h2_expr_compare(node->node, kBoolType, node->compare, lhs, rhs);
}

static h2_t *sema_compare(h2_t *sema, pl0_t *node)
{
    switch (node->type)
    {
    case ePl0Odd:
        return sema_odd(sema, node);
    case ePl0Compare:
        return sema_comp(sema, node);
    default:
        report(sema->reports, eInternal, node->node, "sema-compare: %d", node->type);
        return h2_error(node->node, "sema-compare");
    }
}

static void sema_proc(h2_t *sema, h2_t *hlir, pl0_t *node)
{
    size_t nlocals = vector_len(node->locals);
    size_t sizes[ePl0TagTotal] = {[ePl0TagValues] = nlocals};

    h2_t *nest = h2_module(sema, node->node, node->name, ePl0TagTotal, sizes);

    for (size_t i = 0; i < nlocals; i++)
    {
        pl0_t *local = vector_get(node->locals, i);
        h2_t *it = h2_decl_local(local->node, local->name, kIntType);
        set_var(nest, ePl0TagValues, local->name, it);
        h2_add_local(hlir, it);
    }

    h2_t *ret = h2_stmt_return(node->node, h2_expr_unit(node->node, kVoidType));

    h2_t *inner = sema_vector(nest, node->node, node->body);

    vector_t *body = vector_new(2);
    vector_push(&body, inner);
    vector_push(&body, ret);

    // make sure we have a return statement
    h2_t *stmts = h2_stmt_block(node->node, body);

    h2_close_function(hlir, stmts);
}

static void resolve_global(h2_cookie_t *cookie, h2_t *sema, h2_t *decl, void *user)
{
    h2_close_global(decl, sema_global(sema, user));
}

static void resolve_proc(h2_cookie_t *cookie, h2_t *sema, h2_t *decl, void *user)
{
    sema_proc(sema, decl, user);
}

static void insert_module(h2_t *sema, h2_t *other)
{
    map_iter_t otherValues = map_iter(h2_module_tag(other, ePl0TagValues));
    map_iter_t otherProcs = map_iter(h2_module_tag(other, ePl0TagProcs));

    while (map_has_next(&otherValues))
    {
        h2_t *decl = map_next(&otherValues).value;
        if (!h2_has_vis(decl, eVisiblePublic)) continue;

        set_var(sema, ePl0TagImportedValues, h2_get_name(decl), decl);
    }

    while (map_has_next(&otherProcs))
    {
        h2_t *decl = map_next(&otherProcs).value;
        if (!h2_has_vis(decl, eVisiblePublic)) continue;

        set_proc(sema, ePl0TagImportedProcs, h2_get_name(decl), decl);
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
    h2_cookie_t *cookie = lifetime_get_cookie(lifetime);

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

    h2_t *sema = h2_module_root(reports, cookie, root->node, id, ePl0TagTotal, sizes);

    // forward declare everything
    for (size_t i = 0; i < totalConsts; i++)
    {
        pl0_t *it = vector_get(root->consts, i);

        h2_resolve_info_t resolve = {
            .sema = sema,
            .user = it,
            .fnResolve = resolve_global
        };

        h2_t *hlir = h2_open_global(it->node, it->name, kConstType, resolve);
        h2_set_attrib(hlir, &kExportAttrib);

        set_var(sema, ePl0TagValues, it->name, hlir);
    }

    for (size_t i = 0; i < totalGlobals; i++)
    {
        pl0_t *it = vector_get(root->globals, i);

        h2_resolve_info_t resolve = {
            .sema = sema,
            .user = it,
            .fnResolve = resolve_global
        };

        h2_t *hlir = h2_open_global(it->node, it->name, kIntType, resolve);
        h2_set_attrib(hlir, &kExportAttrib);

        set_var(sema, ePl0TagValues, it->name, hlir);
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        pl0_t *it = vector_get(root->procs, i);

        h2_t *signature = h2_type_closure(it->node, it->name, kVoidType, vector_of(0), eArityFixed);
        h2_resolve_info_t resolve = {
            .sema = sema,
            .user = it,
            .fnResolve = resolve_proc
        };

        h2_t *hlir = h2_open_function(it->node, it->name, signature, resolve);
        h2_set_attrib(hlir, &kExportAttrib);

        set_proc(sema, ePl0TagProcs, it->name, hlir);
    }


    context_update(context, root, sema);
}

void pl0_process_imports(context_t *context)
{
    lifetime_t *lifetime = context_get_lifetime(context);
    pl0_t *root = context_get_ast(context);
    h2_t *sema = context_get_module(context);

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

        h2_t *lib = context_get_module(ctx);

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
    h2_t *mod = context_get_module(context);

    if (root->entry != NULL)
    {
        h2_t *body = sema_stmt(mod, root->entry);

        // this is the entry point, we only support cli entry points in pl/0 for now
        h2_t *signature = h2_type_closure(root->node, h2_get_name(mod), kVoidType, vector_of(0), eArityFixed);
        h2_t *hlir = h2_decl_function(root->node, h2_get_name(mod), signature, vector_of(0), body);
        h2_set_attrib(hlir, &kEntryAttrib);

        set_decl(mod, ePl0TagProcs, h2_get_name(mod), hlir); // TODO: this is a hack
    }
}
