#include "pl0/sema.h"
#include "cthulhu/broker/broker.h"
#include "cthulhu/events/events.h"
#include "memory/memory.h"
#include "pl0/ast.h"

#include "cthulhu/util/util.h"

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

static tree_t *gPrint = NULL; // pl0_print
static tree_t *gRuntimePrint = NULL; // printf

static const tree_attribs_t kPrintAttrib = {
    .link = eLinkImport,
    .visibility = eVisiblePrivate,
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
    arena_t *arena = get_global_arena();
    return str_lower(name, arena);
}

static void report_pl0_shadowing(logger_t *reports, const char *name, const node_t *prev, const node_t *next)
{
    event_builder_t id = evt_symbol_shadowed(reports, name, prev, next);
    msg_note(id, "PL/0 is case insensitive");
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

const pl0_tag_t kVarTags[] = { ePl0TagValues, ePl0TagImportedValues };

static tree_t *get_var(tree_t *sema, const char *name)
{
    return get_decl(sema, name, kVarTags, sizeof(kVarTags) / sizeof(pl0_tag_t));
}

const pl0_tag_t kProcTags[] = { ePl0TagProcs, ePl0TagImportedProcs };

static tree_t *get_proc(tree_t *sema, const char *name)
{
    return get_decl(sema, name, kProcTags, sizeof(kProcTags) / sizeof(pl0_tag_t));
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
        const node_t *other_node = tree_get_node(other);
        report_pl0_shadowing(sema->reports, name, other_node, node);
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
        const node_t *other_node = tree_get_node(other);

        report_pl0_shadowing(sema->reports, name, other_node, node);
        return;
    }

    set_decl(sema, tag, name, tree);
}

static vector_t *make_runtime_path(arena_t *arena)
{
    return str_split("pl0.lang", ".", arena);
}

static tree_t *get_string_type(const node_t *node, size_t size)
{
    return tree_type_pointer(node, "string", gCharType, size);
}

static tree_t *get_bool_type(const node_t *node)
{
    return tree_type_bool(node, "boolean");
}

static tree_storage_t get_const_storage(const tree_t *type)
{
    tree_storage_t storage = {
        .storage = type,
        .length = 1,
        .quals = eQualConst
    };

    return storage;
}

static tree_storage_t get_mutable_storage(const tree_t *type)
{
    tree_storage_t storage = {
        .storage = type,
        .length = 1,
        .quals = eQualMutable
    };

    return storage;
}

void pl0_init(language_runtime_t *runtime, tree_t *root)
{
    const node_t *node = tree_get_node(root);
    arena_t *arena = runtime->arena;

    gIntType = tree_type_digit(node, "integer", eDigitInt, eSignSigned);
    gCharType = tree_type_digit(node, "char", eDigitChar, eSignSigned);
    gBoolType = get_bool_type(node);

    gVoidType = tree_type_unit(node, "void");

    gIntRef = tree_type_reference(node, "ref", gIntType);

    tree_t *string_type = get_string_type(node, 4);

    vector_t *params = vector_of(1, arena);
    vector_set(params, 0, tree_decl_param(node, "fmt", string_type));

    tree_t *signature = tree_type_closure(node, "printf", gIntType, params, eArityVariable);
    gRuntimePrint = tree_decl_function(node, "printf", signature, params, &kEmptyVector, NULL);
    tree_set_attrib(gRuntimePrint, &kPrintAttrib);

    tree_t *param = tree_decl_param(node, "number", gIntType);
    vector_t *rt_print_params = vector_init(param, arena);

    vector_t *args = vector_of(2, arena);
    vector_set(args, 0, tree_expr_string(node, string_type, "%d\n", 4));
    vector_set(args, 1, param);
    tree_t *call = tree_expr_call(node, gRuntimePrint, args);

    tree_t *putd_signature = tree_type_closure(node, "pl0_print", gVoidType, rt_print_params, eArityFixed);
    gPrint = tree_decl_function(node, "pl0_print", putd_signature, rt_print_params, &kEmptyVector, call);
    tree_set_attrib(gPrint, &kExportAttrib);

    // populate builtins
    set_proc(root, ePl0TagProcs, "pl0_print", gPrint);
    set_proc(root, ePl0TagProcs, "printf", gRuntimePrint);
}

static void report_pl0_unresolved(logger_t *reports, const node_t *node, const char *name)
{
    event_builder_t id = msg_notify(reports, &kEvent_SymbolNotFound, node, "unresolved reference to `%s`", name);
    msg_note(id, "symbol resolution is case sensitive");
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
        return tree_error(node->node, &kEvent_SymbolNotFound, "unresolved identifier `%s`", node->ident);
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
        NEVER("sema-expr: %d", node->type);
    }
}

static tree_t *sema_vector(tree_t *sema, node_t *node, vector_t *body)
{
    size_t len = vector_len(body);
    arena_t *arena = get_global_arena();
    vector_t *result = vector_of(len, arena);

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
        return tree_error(node->node, &kEvent_FunctionNotFound, "unresolved procedure `%s`", node->procedure);
    }

    return tree_expr_call(node->node, proc, &kEmptyVector);
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
        return tree_error(node->node, &kEvent_VariableNotFound, "unresolved destination variable `%s`", node->dst);
    }

    const tree_t *dst_type = tree_get_type(dst);
    quals_t quals = tree_ty_get_quals(dst_type);

    if (quals & eQualConst)
    {
        msg_notify(sema->reports, &kEvent_MutationOfConst, node->node, "cannot assign to constant value");
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

    arena_t *arena = get_global_arena();
    vector_t *args = vector_init(expr, arena);

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
    default: NEVER("sema-stmt: %d", node->type);
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
    // desugar odd(n) -> n % 2 == 1
    mpz_t two;
    mpz_init_set_ui(two, 2);

    mpz_t one;
    mpz_init_set_ui(one, 1);

    tree_t *val = sema_expr(sema, node->operand);
    tree_t *two_value = tree_expr_digit(node->node, gIntType, two);
    tree_t *one_value = tree_expr_digit(node->node, gIntType, one);
    tree_t *rem = tree_expr_binary(node->node, gIntType, eBinaryRem, val, two_value);
    tree_t *eq = tree_expr_compare(node->node, gBoolType, eCompareEq, rem, one_value);

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
    default: NEVER("sema-compare: %d", node->type);
    }
}

static void sema_proc(tree_t *sema, tree_t *tree, pl0_t *node)
{
    size_t len = vector_len(node->locals);
    size_t sizes[ePl0TagTotal] = {[ePl0TagValues] = len};

    tree_t *nest = tree_module(sema, node->node, node->name, ePl0TagTotal, sizes);

    const tree_storage_t storage = get_mutable_storage(gIntType);

    for (size_t i = 0; i < len; i++)
    {
        pl0_t *local = vector_get(node->locals, i);
        tree_t *it = tree_decl_local(local->node, local->name, storage, gIntRef);
        set_var(nest, ePl0TagValues, local->name, it);
        tree_add_local(tree, it);
    }

    tree_t *ret = tree_stmt_return(node->node, tree_expr_unit(node->node, gVoidType));

    tree_t *inner = sema_vector(nest, node->node, node->body);

    arena_t *arena = get_global_arena();
    vector_t *body = vector_new(2, arena);
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
    map_iter_t other_values = map_iter(tree_module_tag(other, ePl0TagValues));
    map_iter_t other_procs = map_iter(tree_module_tag(other, ePl0TagProcs));

    const char *name = NULL;
    tree_t *decl = NULL;

    while (CTU_MAP_NEXT(&other_values, &name, &decl))
    {
        if (!tree_has_vis(decl, eVisiblePublic)) continue;

        set_var(sema, ePl0TagImportedValues, tree_get_name(decl), decl);
    }

    while (CTU_MAP_NEXT(&other_procs, &name, &decl))
    {
        if (!tree_has_vis(decl, eVisiblePublic)) continue;

        set_proc(sema, ePl0TagImportedProcs, tree_get_name(decl), decl);
    }
}

typedef struct {
    vector_t *consts;
    vector_t *globals;
    vector_t *procs;
} sema_data_t;

void pl0_forward_decls(language_runtime_t *runtime, compile_unit_t *unit)
{
    pl0_t *ast = unit_get_ast(unit);
    size_t const_count = vector_len(ast->consts);
    size_t global_count = vector_len(ast->globals);
    size_t proc_count = vector_len(ast->procs);

    const char *id = vector_len(ast->mod) > 0
        ? vector_tail(ast->mod)
        : unit->name;

    size_t sizes[ePl0TagTotal] = {
        [ePl0TagValues] = const_count + global_count,
        [ePl0TagProcs] = proc_count,
        [ePl0TagImportedValues] = 64,
        [ePl0TagImportedProcs] = 64
    };

    tree_t *root = tree_module(runtime->root, ast->node, id, ePl0TagTotal, sizes);

    const tree_storage_t const_storage = get_const_storage(gIntType);

    // forward declare everything
    for (size_t i = 0; i < const_count; i++)
    {
        pl0_t *it = vector_get(ast->consts, i);

        tree_resolve_info_t resolve = {
            .sema = root,
            .user = it,
            .fn_resolve = resolve_global
        };

        tree_t *tree = tree_open_global(it->node, it->name, gIntRef, resolve);
        tree_set_storage(tree, const_storage);
        tree_set_attrib(tree, &kExportAttrib);

        set_var(root, ePl0TagValues, it->name, tree);
    }

    for (size_t i = 0; i < global_count; i++)
    {
        pl0_t *it = vector_get(ast->globals, i);

        tree_resolve_info_t resolve = {
            .sema = root,
            .user = it,
            .fn_resolve = resolve_global
        };

        tree_t *tree = tree_open_global(it->node, it->name, gIntRef, resolve);
        tree_set_storage(tree, const_storage);
        tree_set_attrib(tree, &kExportAttrib);

        set_var(root, ePl0TagValues, it->name, tree);
    }

    for (size_t i = 0; i < proc_count; i++)
    {
        pl0_t *it = vector_get(ast->procs, i);

        tree_t *signature = tree_type_closure(it->node, it->name, gVoidType, &kEmptyVector, eArityFixed);
        tree_resolve_info_t resolve = {
            .sema = root,
            .user = it,
            .fn_resolve = resolve_proc
        };

        tree_t *tree = tree_open_function(it->node, it->name, signature, resolve);
        tree_set_attrib(tree, &kExportAttrib);

        set_proc(root, ePl0TagProcs, it->name, tree);
    }

    unit_update(unit, ast, root);
}

void pl0_process_imports(language_runtime_t *runtime, compile_unit_t *unit)
{
    pl0_t *ast = unit_get_ast(unit);
    arena_t *arena = runtime->arena;
    tree_t *root = unit->tree;

    size_t import_count = vector_len(ast->imports);
    for (size_t i = 0; i < import_count; i++)
    {
        pl0_t *import_decl = vector_get(ast->imports, i);
        CTASSERT(import_decl->type == ePl0Import);

        unit_id_t id = build_unit_id(import_decl->path, arena);
        compile_unit_t *imported = lang_get_unit(runtime, id);

        if (imported == NULL)
        {
            msg_notify(root->reports, &kEvent_ImportNotFound, import_decl->node, "cannot import `%s`, failed to find module", str_join(".", import_decl->path, arena));
            continue;
        }

        if (imported->tree == root)
        {
            msg_notify(root->reports, &kEvent_CirclularImport, import_decl->node, "module cannot import itself");
            continue;
        }

        insert_module(root, imported->tree);
    }
}

void pl0_compile_module(language_runtime_t *runtime, compile_unit_t *unit)
{
    pl0_t *ast = unit_get_ast(unit);

    if (ast->entry != NULL)
    {
        tree_t *mod = unit->tree;
        const char *name = unit->name;

        tree_t *body = sema_stmt(mod, ast->entry);
        vector_push(&body->stmts, tree_stmt_return(ast->node, tree_expr_unit(ast->node, gVoidType)));

        // this is the entry point, we only support cli entry points in pl/0 for now
        tree_t *signature = tree_type_closure(ast->node, name, gVoidType, &kEmptyVector, eArityFixed);
        tree_t *entry = tree_decl_function(ast->node, name, signature, &kEmptyVector, &kEmptyVector, body);
        tree_set_attrib(entry, &kEntryAttrib);

        // TODO: this is a hack until we support anonymous declarations
        set_decl(mod, ePl0TagProcs, name, entry);
    }
}
