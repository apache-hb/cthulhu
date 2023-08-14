#include "common.h"

#include "cthulhu/mediator/interface.h"
#include "cthulhu/mediator/driver.h"

#include "base/memory.h"
#include "base/panic.h"

#include "scan/scan.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/set.h"

#include "report/report.h"

#include "stacktrace/stacktrace.h"

#include "cthulhu/hlir/h2.h"
#include "cthulhu/hlir/query.h"
#include "cthulhu/hlir/check.h"

static h2_cookie_t *cookie_new(reports_t *reports)
{
    h2_cookie_t *self = ctu_malloc(sizeof(h2_cookie_t));
    self->reports = reports;
    self->stack = vector_new(16);
    return self;
}

static void runtime_init(void)
{
    GLOBAL_INIT();

    stacktrace_init();
    init_gmp(&globalAlloc);
}

static const language_t *add_language_extension(lifetime_t *lifetime, const char *ext, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ext != NULL);
    CTASSERT(lang != NULL);

    const language_t *old = map_get(lifetime->extensions, ext);
    if (old != NULL)
    {
        return old;
    }

    map_set(lifetime->extensions, ext, (void*)lang);
    return NULL;
}

static driver_t *handle_new(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    driver_t *self = ctu_malloc(sizeof(driver_t));

    self->parent = lifetime;
    self->lang = lang;

    return self;
}

bool context_requires_compiling(const context_t *ctx)
{
    return ctx->ast != NULL;
}

lifetime_t *handle_get_lifetime(driver_t *handle)
{
    CTASSERT(handle != NULL);

    return handle->parent;
}

mediator_t *mediator_new(const char *id, version_info_t version)
{
    CTASSERT(id != NULL);

    runtime_init();

    mediator_t *self = ctu_malloc(sizeof(mediator_t));

    self->id = id;
    self->version = version;

    return self;
}

lifetime_t *lifetime_new(mediator_t *mediator)
{
    CTASSERT(mediator != NULL);

    lifetime_t *self = ctu_malloc(sizeof(lifetime_t));

    self->parent = mediator;

    self->reports = begin_reports();

    self->extensions = map_new(16);
    self->modules = map_new(64);

    self->cookie = cookie_new(self->reports);

    return self;
}

const char *stage_to_string(compile_stage_t stage)
{
#define STAGE(ID, STR) case ID: return STR;
    switch (stage)
    {
#include "cthulhu/mediator/mediator.inc"
    default: return "unknown";
    }
}

void lifetime_config_language(lifetime_t *lifetime, ap_t *ap, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ap != NULL);
    CTASSERT(lang != NULL);

    if (lang->fnConfig == NULL) { return; }

    EXEC(lang, fnConfig, lifetime, ap);
}

void lifetime_add_language(lifetime_t *lifetime, const language_t *lang)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);

    CTASSERTF(lang->fnCreate != NULL, "language `%s` has no create function", lang->id);

    for (size_t i = 0; lang->exts[i] != NULL; i++)
    {
        const language_t *old = add_language_extension(lifetime, lang->exts[i], lang);
        if (old == NULL)
        {
            continue;
        }

        report(lifetime->reports, eInternal, node_invalid(), "language `%s` registered under extension `%s` clashes with previously registered language `%s`", lang->id, lang->exts[i], old->id); // TODO: handle this
    }

    driver_t *handle = handle_new(lifetime, lang);
    EXEC(lang, fnCreate, handle);
}

const language_t *lifetime_add_extension(lifetime_t *lifetime, const char *ext, const language_t *lang)
{
    return add_language_extension(lifetime, ext, lang);
}

const language_t *lifetime_get_language(lifetime_t *lifetime, const char *ext)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(ext != NULL);

    return map_get(lifetime->extensions, ext);
}

void lifetime_parse(lifetime_t *lifetime, const language_t *lang, io_t *io)
{
    CTASSERT(lifetime != NULL);
    CTASSERT(lang != NULL);
    CTASSERT(io != NULL);

    CTASSERT(lang->fnParse != NULL);

    scan_t *scan = scan_io(lifetime->reports, lang->id, io);
    driver_t *handle = handle_new(lifetime, lang);

    lang->fnParse(handle, scan);
}

static void resolve_tag(h2_t *mod, size_t tag)
{
    map_iter_t iter = map_iter(h2_module_tag(mod, tag));
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        h2_resolve(h2_get_cookie(mod), entry.value);
    }
}

static void resolve_decls(context_t *ctx)
{
    h2_t *mod = context_get_module(ctx);
    resolve_tag(mod, eSema2Values);
    resolve_tag(mod, eSema2Types);
    resolve_tag(mod, eSema2Procs);
}

void lifetime_resolve(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;

        CTASSERT(ctx != NULL);

        resolve_decls(ctx);
    }
}

void lifetime_run_stage(lifetime_t *lifetime, compile_stage_t stage)
{
    CTASSERT(lifetime != NULL);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;

        CTASSERT(ctx != NULL);

        const language_t *lang = ctx->lang;
        compile_pass_t fnPass = lang->fnCompilePass[stage];

        if (!context_requires_compiling(ctx) || fnPass == NULL)
        {
            continue;
        }

        fnPass(ctx);
    }
}

map_t *lifetime_get_modules(lifetime_t *lifetime)
{
    CTASSERT(lifetime != NULL);

    map_t *mods = map_optimal(64);

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;
        const char *name = entry.key;

        CTASSERTF(ctx != NULL, "module `%s` is NULL", name);
        CTASSERTF(ctx->root != NULL, "module `%s` has NULL root", name);

        map_set(mods, name, ctx->root);
    }

    return mods;
}

typedef struct check_t {
    reports_t *reports;

    const h2_t *cliEntryPoint;
    const h2_t *guiEntryPoint;

    vector_t *exprStack;
    vector_t *typeStack;

    set_t *checkedExprs;
    set_t *checkedTypes;
} check_t;

static void check_ident(check_t *check, const h2_t *decl)
{
    const char *id = h2_get_name(decl);
    CTASSERT(id != NULL);
    CTASSERT(!str_equal(id, ""));

    logverbose("check %s", id);
}

static void check_global_attribs(check_t *check, const h2_t *global)
{
    const h2_attrib_t *attribs = h2_get_attrib(global);
    switch (attribs->link)
    {
    case eLinkImport:
        if (global->global != NULL)
        {
            message_t *id = report(check->reports, eWarn, h2_get_node(global),
                "global `%s` is marked as imported but has an implementation",
                h2_get_name(global)
            );
            report_note(id, "implementation will be ignored");
        }
        break;

    case eLinkModule:
        if (attribs->mangle != NULL)
        {
            message_t *id = report(check->reports, eWarn, h2_get_node(global),
                "global `%s` has internal linkage and user defined mangling",
                h2_get_name(global)
            );
            report_note(id, "attribute will be ignored");
        }
        break;

    case eLinkEntryGui:
    case eLinkEntryCli:
        report(check->reports, eFatal, h2_get_node(global),
            "global `%s` is marked as an entry point but is not a function",
            h2_get_name(global)
        );
        break;

    default: break;
    }
}

static void check_func_attribs(check_t *check, const h2_t *fn)
{
    const h2_attrib_t *attribs = h2_get_attrib(fn);

    switch (attribs->link)
    {
    case eLinkImport:
        if (fn->body != NULL)
        {
            message_t *id = report(check->reports, eWarn, h2_get_node(fn),
                "function `%s` is marked as imported but has an implementation",
                h2_get_name(fn)
            );
            report_note(id, "implementation will be ignored");
        }
        break;

    case eLinkModule:
        if (attribs->mangle != NULL)
        {
            message_t *id = report(check->reports, eWarn, h2_get_node(fn),
                "function `%s` has internal linkage and user defined mangling",
                h2_get_name(fn)
            );
            report_note(id, "attribute will be ignored");
        }
        break;

    case eLinkEntryCli:
        if (check->cliEntryPoint != NULL)
        {
            message_t *id = report(check->reports, eFatal, h2_get_node(fn),
                "multiple CLI entry points defined"
            );
            report_append(id, h2_get_node(check->cliEntryPoint), "previous entry point defined here");
        }
        else
        {
            check->cliEntryPoint = fn;
        }
        break;
    case eLinkEntryGui:
        if (check->guiEntryPoint != NULL)
        {
            message_t *id = report(check->reports, eFatal, h2_get_node(fn),
                "multiple GUI entry points defined"
            );
            report_append(id, h2_get_node(check->guiEntryPoint), "previous entry point defined here");
        }
        else
        {
            check->guiEntryPoint = fn;
        }
        break;

    default: break;
    }
}

static void check_global_recursion(check_t *check, const h2_t *global);

static void check_expr_recursion(check_t *check, const h2_t *tree)
{
    switch (h2_get_kind(tree))
    {
    case eHlir2ExprEmpty:
    case eHlir2ExprUnit:
    case eHlir2ExprBool:
    case eHlir2ExprDigit:
    case eHlir2ExprString:
        break;

    case eHlir2ExprLoad:
        check_expr_recursion(check, tree->load);
        break;

    case eHlir2ExprCall: {
        check_expr_recursion(check, tree->callee);
        size_t len = vector_len(tree->args);
        for (size_t i = 0; i < len; ++i)
        {
            const h2_t *arg = vector_get(tree->args, i);
            check_expr_recursion(check, arg);
        }
        break;
    }

    case eHlir2ExprBinary:
        check_expr_recursion(check, tree->lhs);
        check_expr_recursion(check, tree->rhs);
        break;

    case eHlir2ExprUnary:
        check_expr_recursion(check, tree->operand);
        break;

    case eHlir2ExprCompare:
        check_expr_recursion(check, tree->lhs);
        check_expr_recursion(check, tree->rhs);
        break;

    case eHlir2DeclGlobal:
        check_global_recursion(check, tree);
        break;
    default: NEVER("invalid node kind %s (check-tree-recursion)", h2_to_string(tree));
    }
}

static void check_global_recursion(check_t *check, const h2_t *global)
{
    if (set_contains_ptr(check->checkedExprs, global))
    {
        return;
    }

    size_t idx = vector_find(check->exprStack, global);
    if (idx == SIZE_MAX)
    {
        if (global->global != NULL)
        {
            vector_push(&check->exprStack, (h2_t*)global);
            check_expr_recursion(check, global->global);
            vector_drop(check->exprStack);
        }
    }
    else
    {
        message_t *id = report(check->reports, eFatal, h2_get_node(global),
            "evaluation of `%s` may be infinite",
            h2_get_name(global)
        );
        size_t len = vector_len(check->exprStack);
        for (size_t i = 0; i < len; i++)
        {
            const h2_t *decl = vector_get(check->exprStack, i);
            report_append(id, h2_get_node(decl), "call to `%s`", h2_get_name(decl));
        }
    }

    set_add_ptr(check->checkedExprs, global);
}

///
/// recursive struct checking
///

static void check_struct_recursion(check_t *check, const h2_t *type);

static void check_struct_type_recursion(check_t *check, const h2_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eHlir2TypeBool:
    case eHlir2TypeDigit:
    case eHlir2TypeString:
    case eHlir2TypeUnit:
    case eHlir2TypeEmpty:
    case eHlir2TypePointer:
        break;

    case eHlir2TypeStruct:
        check_struct_recursion(check, type);
        break;

    default: NEVER("invalid type kind %s (check-type-size)", h2_to_string(type));
    }
}

static void check_struct_recursion(check_t *check, const h2_t *type)
{
    if (set_contains_ptr(check->checkedTypes, type))
    {
        return;
    }

    size_t idx = vector_find(check->typeStack, type);
    if (idx == SIZE_MAX)
    {
        vector_push(&check->typeStack, (h2_t*)type);
        size_t len = vector_len(type->fields);
        for (size_t i = 0; i < len; i++)
        {
            const h2_t *field = vector_get(type->fields, i);
            check_struct_type_recursion(check, field->type);
        }
        vector_drop(check->typeStack);
    }
    else
    {
        message_t *id = report(check->reports, eFatal, h2_get_node(type),
            "size of type `%s` may be infinite",
            h2_get_name(type)
        );
        size_t len = vector_len(check->typeStack);
        for (size_t i = 0; i < len; i++)
        {
            const h2_t *decl = vector_get(check->typeStack, i);
            report_append(id, h2_get_node(decl), "call to `%s`", h2_get_name(decl));
        }
    }

    set_add_ptr(check->checkedTypes, type);
}

///
/// recursive pointer checking
///

static void check_type_recursion(check_t *check, const h2_t *type);

static void check_inner_type_recursion(check_t *check, const h2_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eHlir2TypeBool:
    case eHlir2TypeDigit:
    case eHlir2TypeString:
    case eHlir2TypeUnit:
    case eHlir2TypeEmpty:
    case eHlir2TypeStruct:
        break;

    case eHlir2TypePointer:
        check_type_recursion(check, type->pointer);
        break;

    default: NEVER("invalid type kind `%s` (check-type-size)", h2_to_string(type));
    }
}

static void check_type_recursion(check_t *check, const h2_t *type)
{
    if (set_contains_ptr(check->checkedTypes, type))
    {
        return;
    }

    size_t idx = vector_find(check->typeStack, type);
    if (idx == SIZE_MAX)
    {
        vector_push(&check->typeStack, (h2_t*)type);
        check_inner_type_recursion(check, type);
        vector_drop(check->typeStack);
    }
    else
    {
        message_t *id = report(check->reports, eFatal, h2_get_node(type),
            "type `%s` contains an impossible type",
            h2_get_name(type)
        );
        size_t len = vector_len(check->typeStack);
        for (size_t i = 0; i < len; i++)
        {
            const h2_t *decl = vector_get(check->typeStack, i);
            report_append(id, h2_get_node(decl), "call to `%s`", h2_get_name(decl));
        }
    }

    set_add_ptr(check->checkedTypes, type);
}

static void check_any_type_recursion(check_t *check, const h2_t *type)
{
    switch (type->kind)
    {
    case eHlir2TypeStruct:
        check_struct_recursion(check, type);
        break;

    default:
        check_type_recursion(check, type);
        break;
    }
}

static void check_module_valid(check_t *check, const h2_t *mod)
{
    CTASSERT(check != NULL);
    CTASSERT(h2_is(mod, eHlir2DeclModule));

    logverbose("check %s", h2_get_name(mod));

    vector_t *modules = map_values(h2_module_tag(mod, eSema2Modules));
    size_t totalModules = vector_len(modules);
    for (size_t i = 0; i < totalModules; i++)
    {
        const h2_t *child = vector_get(modules, i);
        check_module_valid(check, child);
    }

    vector_t *globals = map_values(h2_module_tag(mod, eSema2Values));
    size_t totalGlobals = vector_len(globals);
    for (size_t i = 0; i < totalGlobals; i++)
    {
        const h2_t *global = vector_get(globals, i);
        CTASSERT(h2_is(global, eHlir2DeclGlobal));
        check_ident(check, global);

        check_global_attribs(check, global);
        check_global_recursion(check, global);
    }

    vector_t *functions = map_values(h2_module_tag(mod, eSema2Procs));
    size_t totalFunctions = vector_len(functions);
    for (size_t i = 0; i < totalFunctions; i++)
    {
        const h2_t *function = vector_get(functions, i);
        CTASSERT(h2_is(function, eHlir2DeclFunction));
        check_ident(check, function);

        check_func_attribs(check, function);
    }

    vector_t *types = map_values(h2_module_tag(mod, eSema2Types));
    size_t totalTypes = vector_len(types);
    for (size_t i = 0; i < totalTypes; i++)
    {
        const h2_t *type = vector_get(types, i);
        // check_ident(check, type); TODO: check these properly

        // nothing else can be recursive (TODO: for now)
        check_any_type_recursion(check, type);
    }
}

void lifetime_check(lifetime_t *lifetime)
{
    check_t check = {
        .reports = lifetime_get_reports(lifetime),

        .exprStack = vector_new(64),
        .typeStack = vector_new(64),

        .checkedExprs = set_new(64),
        .checkedTypes = set_new(64)
    };

    map_iter_t iter = map_iter(lifetime->modules);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        context_t *ctx = entry.value;
        const char *name = entry.key;

        CTASSERTF(ctx != NULL, "module `%s` is NULL", name);
        CTASSERTF(ctx->root != NULL, "module `%s` has NULL root", name);

        // TODO: required checks
        // 0. no nodes are of the wrong type
        // 1. no identifiers are invalid
        // 1. only one 1 entry point of each type is present
        // 2. no types can have infinite size
        // 3. no global types may have circlic dependencies
        // 4. make sure all return statements match the return type of a global or function
        // 5. make sure all operands to nodes are the correct type

        check_module_valid(&check, ctx->root);

        //h2_check(lifetime_get_reports(lifetime), ctx->root);
    }
}
