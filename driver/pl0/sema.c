#include "sema.h"

#include "ast.h"
#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/sema.h"
#include "cthulhu/hlir/type.h"
#include "cthulhu/interface/runtime.h"
#include "report/report-ext.h"

#include "base/macros.h"
#include "base/util.h"
#include "std/str.h"

static hlir_t *kIntegerType;
static hlir_t *kBoolType;
static hlir_t *kStringType;
static hlir_t *kVoidType;

static hlir_t *kPrint;
static hlir_t *kFmtString;

static const hlir_attributes_t *kExported;

static const hlir_attributes_t *kConst;
static const hlir_attributes_t *kMutable;

void pl0_init(runtime_t *runtime)
{
    UNUSED(runtime);

    const node_t node = node_builtin();

    kExported = hlir_linkage(eLinkExported);

    kConst = hlir_attributes(eLinkExported, eQualConst, NULL);
    kMutable = hlir_attributes(eLinkExported, DEFAULT_TAGS, NULL);

    kIntegerType = hlir_digit(node, "integer", eDigitInt, eSigned);
    kBoolType = hlir_bool(node, "boolean");
    kStringType = hlir_string(node, "string");
    kVoidType = hlir_void(node, "void");

    kFmtString = hlir_string_literal(node, kStringType, "%d\n", 3);

    const hlir_attributes_t *printAttributes = hlir_attributes(eLinkImported, DEFAULT_TAGS, "printf");
    signature_t signature = {.params = vector_init(kStringType), .result = kIntegerType, .variadic = true};
    kPrint = hlir_function(node, "printf", signature, vector_of(0), NULL);
    hlir_set_attributes(kPrint, printAttributes);
}

typedef enum
{
    eTagValues, // hlir_t*
    eTagConsts, // hlir_t*
    eTagProcs,  // hlir_t*

    eTagMax
} tag_t;

static void report_pl0_shadowing(reports_t *reports, const char *name, node_t prevDefinition, node_t newDefinition)
{
    message_t *id = report_shadow(reports, name, prevDefinition, newDefinition);
    report_note(id, "PL/0 is case insensitive");
}

static void report_pl0_unresolved(reports_t *reports, const node_t node, const char *name)
{
    report(reports, eFatal, node, "unresolved reference to `%s`", name);
}

static hlir_t *get_var(sema_t *sema, const char *name)
{
    hlir_t *constHlir = sema_get(sema, eTagConsts, name);
    if (constHlir != NULL)
    {
        return constHlir;
    }

    hlir_t *varHlir = sema_get(sema, eTagValues, name);
    if (varHlir != NULL)
    {
        return varHlir;
    }

    return NULL;
}

static void set_proc(sema_t *sema, const char *name, hlir_t *proc)
{
    hlir_t *other = sema_get(sema, eTagProcs, name);
    if (other != NULL && other != proc)
    {
        const node_t node = get_hlir_node(proc);
        const node_t otherNode = get_hlir_node(other);
        report_pl0_shadowing(sema->reports, name, otherNode, node);
        return;
    }

    sema_set(sema, eTagProcs, name, proc);
}

static hlir_t *get_proc(sema_t *sema, const char *name)
{
    return sema_get(sema, eTagProcs, name);
}

static void set_var(sema_t *sema, size_t tag, const char *name, hlir_t *hlir)
{
    hlir_t *other = get_var(sema, name);
    if (other != NULL && other != hlir)
    {
        const node_t node = get_hlir_node(hlir);
        const node_t otherNode = get_hlir_node(other);

        report_pl0_shadowing(sema->reports, name, otherNode, node);
        return;
    }

    sema_set(sema, tag, name, hlir);
}

static hlir_t *sema_expr(sema_t *sema, pl0_t *node);
static hlir_t *sema_compare(sema_t *sema, pl0_t *node);
static hlir_t *sema_stmt(sema_t *sema, pl0_t *node);

static hlir_t *sema_digit(pl0_t *node)
{
    return hlir_digit_literal(node->node, kIntegerType, node->digit);
}

static hlir_t *sema_ident(sema_t *sema, pl0_t *node)
{
    hlir_t *var = get_var(sema, node->ident);
    if (var == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->ident);
        return hlir_error(node->node, "unresolved identifier");
    }
    return hlir_name(node->node, var);
}

static hlir_t *sema_binary(sema_t *sema, pl0_t *node)
{
    hlir_t *lhs = sema_expr(sema, node->lhs);
    hlir_t *rhs = sema_expr(sema, node->rhs);
    return hlir_binary(node->node, kIntegerType, node->binary, lhs, rhs);
}

static hlir_t *sema_unary(sema_t *sema, pl0_t *node)
{
    hlir_t *operand = sema_expr(sema, node->operand);
    return hlir_unary(node->node, operand, node->unary);
}

static hlir_t *sema_expr(sema_t *sema, pl0_t *node)
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
        return hlir_error(node->node, "sema-expr");
    }
}

static hlir_t *sema_vector(sema_t *sema, const node_t node, vector_t *body)
{
    size_t len = vector_len(body);
    vector_t *result = vector_of(len);

    for (size_t i = 0; i < len; i++)
    {
        pl0_t *it = vector_get(body, i);
        hlir_t *temp = sema_stmt(sema, it);
        vector_set(result, i, temp);
    }

    return hlir_stmts(node, result);
}

static hlir_t *sema_stmts(sema_t *sema, pl0_t *node)
{
    return sema_vector(sema, node->node, node->stmts);
}

static hlir_t *sema_call(sema_t *sema, pl0_t *node)
{
    hlir_t *proc = get_proc(sema, node->procedure);
    if (proc == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->procedure);
        return hlir_error(node->node, "unresolved procedure");
    }

    vector_t *args = vector_new(0);

    return hlir_call(node->node, proc, args);
}

static hlir_t *sema_branch(sema_t *sema, pl0_t *node)
{
    hlir_t *cond = sema_compare(sema, node->cond);
    hlir_t *then = sema_stmt(sema, node->then);

    return hlir_branch(node->node, cond, then, NULL);
}

static hlir_t *sema_assign(sema_t *sema, pl0_t *node)
{
    hlir_t *dst = get_var(sema, node->dst);
    hlir_t *src = sema_expr(sema, node->src);

    if (dst == NULL)
    {
        report_pl0_unresolved(sema->reports, node->node, node->dst);
        return hlir_error(node->node, "unresolved variable");
    }

    const hlir_attributes_t *attrs = get_hlir_attributes(dst);

    if (attrs->tags & eQualConst)
    {
        report(sema->reports, eFatal, node->node, "cannot assign to constant value");
    }

    return hlir_assign(node->node, dst, src);
}

static hlir_t *sema_loop(sema_t *sema, pl0_t *node)
{
    hlir_t *cond = sema_compare(sema, node->cond);
    hlir_t *body = sema_stmt(sema, node->then);

    return hlir_loop(node->node, cond, body, NULL);
}

static hlir_t *sema_print(sema_t *sema, pl0_t *node)
{
    hlir_t *expr = sema_expr(sema, node->print);

    vector_t *args = vector_of(2);
    vector_set(args, 0, kFmtString);
    vector_set(args, 1, expr);

    return hlir_call(node->node, kPrint, args);
}

static hlir_t *sema_stmt(sema_t *sema, pl0_t *node)
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
        return hlir_error(node->node, "sema-stmt");
    }
}

static hlir_t *sema_global(sema_t *sema, pl0_t *node)
{
    pl0_t *val = node->value;
    if (val == NULL)
    {
        return hlir_int_literal(node->node, kIntegerType, 0);
    }
    else
    {
        return sema_expr(sema, val);
    }
}

static hlir_t *sema_odd(sema_t *sema, pl0_t *node)
{
    hlir_t *val = sema_expr(sema, node->operand);
    hlir_t *two = hlir_int_literal(node->node, kIntegerType, 2);
    hlir_t *one = hlir_int_literal(node->node, kIntegerType, 1);
    hlir_t *rem = hlir_binary(node->node, kIntegerType, eBinaryRem, val, two);
    hlir_t *eq = hlir_compare(node->node, kBoolType, eCompareEq, rem, one);

    return eq;
}

static hlir_t *sema_comp(sema_t *sema, pl0_t *node)
{
    hlir_t *lhs = sema_expr(sema, node->lhs);
    hlir_t *rhs = sema_expr(sema, node->rhs);

    return hlir_compare(node->node, kBoolType, node->compare, lhs, rhs);
}

static hlir_t *sema_compare(sema_t *sema, pl0_t *node)
{
    switch (node->type)
    {
    case ePl0Odd:
        return sema_odd(sema, node);
    case ePl0Compare:
        return sema_comp(sema, node);
    default:
        report(sema->reports, eInternal, node->node, "sema-compare: %d", node->type);
        return hlir_error(node->node, "sema-compare");
    }
}

static void sema_proc(sema_t *sema, hlir_t *hlir, pl0_t *node)
{
    size_t nlocals = vector_len(node->locals);
    size_t sizes[eTagMax] = {[eTagValues] = nlocals};

    sema_t *nest = sema_new(sema, sema->reports, eTagMax, sizes);

    for (size_t i = 0; i < nlocals; i++)
    {
        pl0_t *local = vector_get(node->locals, i);
        hlir_t *it = hlir_local(local->node, local->name, kIntegerType);
        set_var(nest, eTagValues, local->name, it);
        hlir_add_local(hlir, it);
    }

    hlir_t *body = sema_vector(nest, node->node, node->body);

    sema_delete(nest);

    hlir_build_function(hlir, body);
}

static void insert_module(sema_t *sema, sema_t *other)
{
    map_iter_t otherValues = map_iter(sema_tag(other, eTagValues));
    map_iter_t otherConsts = map_iter(sema_tag(other, eTagConsts));
    map_iter_t otherProcs = map_iter(sema_tag(other, eTagProcs));

    while (map_has_next(&otherValues))
    {
        hlir_t *decl = map_next(&otherValues).value;
        set_var(sema, eTagValues, get_hlir_name(decl), decl);
    }

    while (map_has_next(&otherConsts))
    {
        hlir_t *decl = map_next(&otherConsts).value;
        set_var(sema, eTagConsts, get_hlir_name(decl), decl);
    }

    while (map_has_next(&otherProcs))
    {
        hlir_t *decl = map_next(&otherProcs).value;
        set_var(sema, eTagProcs, get_hlir_name(decl), decl);
    }
}

typedef struct
{
    size_t totalConsts;
    size_t totalGlobals;
    size_t totalProcs;

    vector_t *consts;
    vector_t *globals;
    vector_t *procs;
} sema_data_t;

void pl0_forward_decls(runtime_t *runtime, compile_t *compile)
{
    pl0_t *root = compile->ast;

    size_t totalConsts = vector_len(root->consts);
    size_t totalGlobals = vector_len(root->globals);
    size_t totalFunctions = vector_len(root->procs);

    vector_t *consts = vector_new(totalConsts);
    vector_t *globals = vector_new(totalGlobals);
    vector_t *procs = vector_new(totalFunctions);

    scan_t *thisScanner = get_node_scanner(root->node);
    const char *moduleName = root->mod == NULL ? str_filename(scan_path(thisScanner)) : root->mod;
    hlir_t *mod = hlir_module(root->node, moduleName, vector_of(0), vector_of(0), vector_of(0));

    size_t sizes[eTagMax] = {
        [eTagConsts] = totalConsts,
        [eTagValues] = totalGlobals,
        [eTagProcs] = totalFunctions,
    };

    sema_t *sema = sema_new(NULL, runtime->reports, eTagMax, sizes);

    // forward declare everything
    for (size_t i = 0; i < totalConsts; i++)
    {
        pl0_t *it = vector_get(root->consts, i);

        hlir_t *hlir = hlir_begin_global(it->node, it->name, kIntegerType);
        hlir_set_attributes(hlir, kConst);
        hlir_set_parent(hlir, mod);

        set_var(sema, eTagConsts, it->name, hlir);
        vector_push(&consts, hlir);
    }

    for (size_t i = 0; i < totalGlobals; i++)
    {
        pl0_t *it = vector_get(root->globals, i);

        hlir_t *hlir = hlir_begin_global(it->node, it->name, kIntegerType);
        hlir_set_attributes(hlir, kMutable);
        hlir_set_parent(hlir, mod);

        set_var(sema, eTagValues, it->name, hlir);
        vector_push(&globals, hlir);
    }

    for (size_t i = 0; i < totalFunctions; i++)
    {
        pl0_t *it = vector_get(root->procs, i);
        signature_t signature = {.params = vector_of(0), .result = kVoidType, .variadic = false};

        hlir_t *hlir = hlir_begin_function(it->node, it->name, signature);
        hlir_set_attributes(hlir, kExported);
        hlir_set_parent(hlir, mod);

        set_proc(sema, it->name, hlir);
        vector_push(&procs, hlir);
    }

    sema_data_t semaData = {
        .totalConsts = totalConsts,
        .totalGlobals = totalGlobals,
        .totalProcs = totalFunctions,
        .consts = consts,
        .globals = globals,
        .procs = procs,
    };

    sema_set_data(sema, BOX(semaData));

    hlir_update_module(mod, vector_of(0), vector_merge(globals, consts), procs);

    compile->hlir = mod;
    compile->sema = sema;
}

void pl0_process_imports(runtime_t *runtime, compile_t *compile)
{
    pl0_t *root = compile->ast;
    sema_t *sema = compile->sema;

    size_t totalImports = vector_len(root->imports);
    for (size_t i = 0; i < totalImports; i++)
    {
        pl0_t *importDecl = vector_get(root->imports, i);
        const char *pathToImport = importDecl->ident;
        sema_t *lib = find_module(runtime, pathToImport);

        if (lib == NULL)
        {
            report(sema->reports, eFatal, importDecl->node, "cannot import `%s`, failed to find module", pathToImport);
            continue;
        }

        if (lib == sema)
        {
            report(sema->reports, eFatal, importDecl->node, "module cannot import itself");
            continue;
        }

        insert_module(sema, lib);
    }
}

void pl0_compile_module(runtime_t *runtime, compile_t *compile)
{
    UNUSED(runtime);

    pl0_t *root = compile->ast;
    sema_t *sema = compile->sema;
    hlir_t *self = compile->hlir;
    sema_data_t *semaData = sema_get_data(sema);

    for (size_t i = 0; i < semaData->totalConsts; i++)
    {
        pl0_t *it = vector_get(root->consts, i);
        hlir_t *hlir = vector_get(semaData->consts, i);
        hlir_build_global(hlir, sema_global(sema, it));
    }

    for (size_t i = 0; i < semaData->totalGlobals; i++)
    {
        pl0_t *it = vector_get(root->globals, i);
        hlir_t *hlir = vector_get(semaData->globals, i);
        hlir_build_global(hlir, sema_global(sema, it));
    }

    for (size_t i = 0; i < semaData->totalProcs; i++)
    {
        pl0_t *it = vector_get(root->procs, i);
        hlir_t *hlir = vector_get(semaData->procs, i);
        sema_proc(sema, hlir, it);
    }

    if (root->entry != NULL)
    {
        hlir_t *body = sema_stmt(sema, root->entry);
        signature_t signature = {
            .params = vector_of(0),
            .result = kVoidType,
            .variadic = false,
        };

        // this is the entry point, we only support cli entry points in pl/0 for now
        const hlir_attributes_t *attribs = hlir_attributes(eLinkEntryCli, DEFAULT_TAGS, NULL);
        const char *modName = get_hlir_name(compile->hlir);

        hlir_t *hlir = hlir_function(root->node, modName, signature, vector_of(0), body);
        hlir_set_attributes(hlir, attribs);
        hlir_set_parent(hlir, self);

        vector_push(&semaData->procs, hlir);
    }

    vector_push(&semaData->procs, kPrint);

    hlir_update_module(self, self->types, vector_merge(semaData->consts, semaData->globals), semaData->procs);
}
