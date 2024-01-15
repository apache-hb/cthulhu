#include "cthulhu/check/check.h"

#include "cthulhu/events/events.h"

#include "cthulhu/util/util.h"
#include "cthulhu/util/type.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "memory/memory.h"
#include "std/vector.h"
#include "std/set.h"
#include "std/map.h"
#include "std/str.h"

#include "base/panic.h"
#include "core/macros.h"

#include <stdint.h>

typedef struct check_t
{
    logger_t *reports;

    const tree_t *cli_entry;
    const tree_t *gui_entry;

    vector_t *expr_stack;
    vector_t *type_stack;

    set_t *checked_exprs;
    set_t *checked_types;
} check_t;

// check for a valid name and a type being set
static bool check_simple(check_t *check, const tree_t *decl)
{
    CTU_UNUSED(check);

    if (tree_is(decl, eTreeError)) { return false; } // TODO: are errors always reported?

    const char *id = tree_get_name(decl);
    CTASSERT(id != NULL);
    CTASSERT(!str_equal(id, ""));

    CTASSERTF(decl->type != NULL, "decl `%s` has no type", id);

    return true;
}

static void check_global_attribs(check_t *check, const tree_t *global)
{
    const tree_attribs_t *attribs = tree_get_attrib(global);
    switch (attribs->link)
    {
    case eLinkImport:
        if (global->initial != NULL)
        {
            event_builder_t id = msg_notify(check->reports, &kEvent_ImportedWithImpl, tree_get_node(global),
                "global `%s` is marked as imported but has an implementation",
                tree_get_name(global)
            );
            msg_note(id, "implementation will be ignored");
        }
        break;

    case eLinkModule:
        if (attribs->mangle != NULL)
        {
            event_builder_t id = msg_notify(check->reports, &kEvent_IgnoredMangling, tree_get_node(global),
                "global `%s` has internal linkage and user defined mangling",
                tree_get_name(global)
            );
            msg_note(id, "attribute will be ignored");
        }
        break;

    case eLinkEntryGui:
    case eLinkEntryCli:
        msg_notify(check->reports, &kEvent_EntryNotFunction, tree_get_node(global),
            "global `%s` is marked as an entry point but is not a function",
            tree_get_name(global)
        );
        break;

    default: break;
    }
}

static void check_func_has_body(check_t *check, const tree_t *fn)
{
    if (fn->body != NULL) { return; }

    msg_notify(check->reports, &kEvent_EntryMissingBody, tree_get_node(fn),
        "function `%s` is an entry point, but has no body",
        tree_get_name(fn)
    );
}

static void check_func_attribs(check_t *check, const tree_t *fn)
{
    const tree_attribs_t *attribs = tree_get_attrib(fn);

    switch (attribs->link)
    {
    case eLinkImport:
        if (fn->body != NULL)
        {
            event_builder_t id = msg_notify(check->reports, &kEvent_ImportedWithImpl, tree_get_node(fn),
                "function `%s` is marked as imported but has an implementation",
                tree_get_name(fn)
            );
            msg_note(id, "implementation will be ignored");
        }
        break;

    case eLinkModule:
        if (attribs->mangle != NULL)
        {
            event_builder_t id = msg_notify(check->reports, &kEvent_IgnoredMangling, tree_get_node(fn),
                "function `%s` has internal linkage and user defined mangling",
                tree_get_name(fn)
            );
            msg_note(id, "attribute will be ignored");
        }
        break;

    case eLinkEntryCli:
        check_func_has_body(check, fn);
        if (check->cli_entry != NULL)
        {
            event_builder_t id = msg_notify(check->reports, &kEvent_MultipleEntryPoints, tree_get_node(fn),
                "multiple CLI entry points defined"
            );
            msg_append(id, tree_get_node(check->cli_entry), "previous entry point defined here");
        }
        else
        {
            check->cli_entry = fn;
        }
        break;
    case eLinkEntryGui:
        check_func_has_body(check, fn);
        if (check->gui_entry != NULL)
        {
            event_builder_t id = msg_notify(check->reports, &kEvent_MultipleEntryPoints, tree_get_node(fn),
                "multiple GUI entry points defined"
            );
            msg_append(id, tree_get_node(check->gui_entry), "previous entry point defined here");
        }
        else
        {
            check->gui_entry = fn;
        }
        break;

    default: break;
    }
}

static void check_func_return_equal(check_t *check, const tree_t *return_type, const tree_t *real_type)
{
    if (util_types_equal(return_type, real_type)) { return; }

    msg_notify(check->reports, &kEvent_ReturnTypeMismatch, tree_get_node(real_type),
        "return type `%s` does not match function return type `%s`",
        tree_to_string(real_type),
        tree_to_string(return_type)
    );
}

static void check_deprecated_call(check_t *check, const tree_t *expr)
{
    const tree_t *fn = expr->callee;
    // cant check if the callee is an indirect call
    if (!tree_is(fn, eTreeDeclFunction)) { return; }

    const tree_attribs_t *attribs = tree_get_attrib(fn);

    if (attribs->deprecated == NULL) { return; }

    event_builder_t id = msg_notify(check->reports, &kEvent_Deprecated, tree_get_node(expr),
        "call to deprecated function `%s`",
        tree_get_name(fn)
    );
    msg_note(id, "deprecated: %s", attribs->deprecated);
}

static const char *get_fn_name(const tree_t *fn)
{
    if (tree_is(fn, eTreeDeclFunction)) { return tree_get_name(fn); }

    // its an indirect call, so give the type name

    // TODO: need a pretty typename function
    return tree_to_string(tree_get_type(fn));
}

static void check_params(check_t *check, vector_t *args, vector_t *params, size_t count, const char *name)
{
    for (size_t i = 0; i < count; i++)
    {
        // at this point the language frontend will have turned
        // any implicit casts into explicit casts, so simple type comparison
        // is enough
        const tree_t *arg = vector_get(args, i);
        const tree_t *param = vector_get(params, i);

        const tree_t *arg_type = tree_get_type(arg);
        const tree_t *param_type = tree_get_type(param);

        if (!util_types_equal(arg_type, param_type))
        {
            event_builder_t id = msg_notify(check->reports, &kEvent_IncorrectParamType, tree_get_node(arg),
                "incorrect type for parameter %zu of function `%s`",
                i + 1,
                name
            );
            msg_note(id, "expected `%s`, got `%s`", tree_to_string(param_type), tree_to_string(arg_type));
        }
    }
}

static void check_call_args_fixed(check_t *check, const tree_t *expr, vector_t *args, vector_t *params)
{
    size_t arg_count = vector_len(args);
    size_t param_count = vector_len(params);

    const char *name = get_fn_name(expr->callee);

    if (arg_count != param_count)
    {
        event_builder_t id = msg_notify(check->reports, &kEvent_IncorrectParamCount, tree_get_node(expr),
            "incorrect number of parameters to function `%s`",
            name
        );
        msg_note(id, "expected %zu, got %zu", param_count, arg_count);
    }

    size_t count = MIN(arg_count, param_count);

    check_params(check, args, params, count, name);
}

static void check_call_args_variadic(check_t *check, const tree_t *expr, vector_t *args, vector_t *params)
{
    size_t arg_count = vector_len(args);
    size_t param_count = vector_len(params);

    const char *name = get_fn_name(expr->callee);

    if (arg_count < param_count)
    {
        event_builder_t id = msg_notify(check->reports, &kEvent_IncorrectParamCount, tree_get_node(expr),
            "incorrect number of parameters to variadic function `%s`",
            name
        );
        msg_note(id, "expected at least %zu parameters, only got %zu", param_count, arg_count);
    }

    size_t count = MIN(arg_count, param_count);

    check_params(check, args, params, count, name);
}

static void check_call_arguments(check_t *check, const tree_t *expr)
{
    const tree_t *fn = expr->callee;
    vector_t *fn_args = expr->args;
    vector_t *fn_params = tree_fn_get_params(fn);
    arity_t arity = tree_fn_get_arity(fn);

    switch (arity)
    {
    case eArityFixed:
        check_call_args_fixed(check, expr, fn_args, fn_params);
        break;

    case eArityVariable:
        check_call_args_variadic(check, expr, fn_args, fn_params);
        break;

    default:
        NEVER("invalid arity %d (check-call-arguments)", arity);
    }
}

static void check_func_body(check_t *check, const tree_t *return_type, const tree_t *stmt)
{
    switch (stmt->kind)
    {
    case eTreeStmtBlock:
        for (size_t i = 0; i < vector_len(stmt->stmts); i++)
        {
            check_func_body(check, return_type, vector_get(stmt->stmts, i));
        }
        break;

    case eTreeStmtLoop:
    case eTreeStmtBranch:
    case eTreeStmtAssign:
        break;

    case eTreeStmtReturn:
        check_func_return_equal(check, return_type, tree_get_type(stmt->value));
        break;

    case eTreeExprCompare:
    case eTreeExprBinary:
    case eTreeExprLoad:
        break; // TODO: check

    case eTreeExprCall:
        check_deprecated_call(check, stmt);
        check_call_arguments(check, stmt);
        break;

    default:
        NEVER("invalid statement kind %s (check-func-body)", tree_to_string(stmt));
    }
}

static bool will_always_return(const tree_t *stmt)
{
    CTASSERT(stmt != NULL);

    switch (stmt->kind)
    {
    case eTreeStmtReturn:
        return true;

    case eTreeStmtBranch: {
        // if the other branch is null then this branch may not always return
        bool other_returns = stmt->other != NULL && will_always_return(stmt->other);
        return will_always_return(stmt->then) && other_returns;
    }

    case eTreeStmtLoop:
        return will_always_return(stmt->then);

    case eTreeStmtBlock:
        for (size_t i = 0; i < vector_len(stmt->stmts); i++)
        {
            if (will_always_return(vector_get(stmt->stmts, i)))
            {
                return true;
            }
        }
        return false;

    default:
        return false;
    }
}

static void check_func_details(check_t *check, const tree_t *fn)
{
    if (fn->body == NULL) { return; }

    const tree_t *fn_type = tree_get_type(fn);
    const tree_t *return_type = fn_type->result;

    check_func_body(check, return_type, fn->body);

    if (!will_always_return(fn->body))
    {
        if (tree_is(return_type, eTreeTypeUnit)) { return; }

        msg_notify(check->reports, &kEvent_MayNotReturn, tree_get_node(fn),
            "function `%s` may not return a value",
            tree_get_name(fn)
        );
    }
}

static void check_global_recursion(check_t *check, const tree_t *global);

static void check_expr_recursion(check_t *check, const tree_t *tree)
{
    switch (tree_get_kind(tree))
    {
    case eTreeExprEmpty:
    case eTreeExprUnit:
    case eTreeExprBool:
    case eTreeExprDigit:
    case eTreeExprString:
        break;

    case eTreeExprLoad:
        check_expr_recursion(check, tree->load);
        break;

    case eTreeExprCall: {
        check_expr_recursion(check, tree->callee);
        size_t len = vector_len(tree->args);
        for (size_t i = 0; i < len; ++i)
        {
            const tree_t *arg = vector_get(tree->args, i);
            check_expr_recursion(check, arg);
        }
        break;
    }

    case eTreeExprBinary:
        check_expr_recursion(check, tree->lhs);
        check_expr_recursion(check, tree->rhs);
        break;

    case eTreeExprUnary:
        check_expr_recursion(check, tree->operand);
        break;

    case eTreeExprCast:
        check_expr_recursion(check, tree->cast);
        break;

    case eTreeExprCompare:
        check_expr_recursion(check, tree->lhs);
        check_expr_recursion(check, tree->rhs);
        break;

    case eTreeDeclGlobal:
        check_global_recursion(check, tree);
        break;
    default: NEVER("invalid node kind %s (check-tree-recursion)", tree_to_string(tree));
    }
}

static void check_global_recursion(check_t *check, const tree_t *global)
{
    if (set_contains(check->checked_exprs, global))
    {
        return;
    }

    size_t idx = vector_find(check->expr_stack, global);
    if (idx == SIZE_MAX)
    {
        if (global->initial != NULL)
        {
            vector_push(&check->expr_stack, (tree_t*)global);
            check_expr_recursion(check, global->initial);
            vector_drop(check->expr_stack);
        }
    }
    else
    {
        event_builder_t id = msg_notify(check->reports, &kEvent_RecursiveEval, tree_get_node(global),
            "evaluation of `%s` may be infinite",
            tree_get_name(global)
        );
        size_t len = vector_len(check->expr_stack);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *decl = vector_get(check->expr_stack, i);
            msg_append(id, tree_get_node(decl), "call to `%s`", tree_get_name(decl));
        }
    }

    set_add(check->checked_exprs, global);
}

///
/// recursive struct checking
///

static void check_aggregate_recursion(check_t *check, const tree_t *type);

static void check_struct_type_recursion(check_t *check, const tree_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eTreeTypeBool:
    case eTreeTypeDigit:
    case eTreeTypeUnit:
    case eTreeTypeEmpty:
    case eTreeTypePointer:
    case eTreeTypeOpaque:
    case eTreeTypeClosure:
        break;

    case eTreeTypeStruct:
    case eTreeTypeUnion:
        check_aggregate_recursion(check, type);
        break;

    default: NEVER("invalid type kind %s (check-type-size)", tree_to_string(type));
    }
}

static void check_aggregate_recursion(check_t *check, const tree_t *type)
{
    if (set_contains(check->checked_types, type))
    {
        return;
    }

    size_t idx = vector_find(check->type_stack, type);
    if (idx == SIZE_MAX)
    {
        vector_push(&check->type_stack, (tree_t*)type);
        size_t len = vector_len(type->fields);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *field = vector_get(type->fields, i);
            check_struct_type_recursion(check, field->type);
        }
        vector_drop(check->type_stack);
    }
    else
    {
        event_builder_t id = msg_notify(check->reports, &kEvent_InfiniteSizedType, tree_get_node(type),
            "size of type `%s` may be infinite",
            tree_get_name(type)
        );
        size_t len = vector_len(check->type_stack);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *decl = vector_get(check->type_stack, i);
            msg_append(id, tree_get_node(decl), "call to `%s`", tree_get_name(decl));
        }
    }

    set_add(check->checked_types, type);
}

///
/// recursive pointer checking
///

static void check_type_recursion(check_t *check, const tree_t *type);

static void check_inner_type_recursion(check_t *check, const tree_t *type)
{
    CTASSERT(type != NULL);

    switch (type->kind)
    {
    case eTreeTypeBool:
    case eTreeTypeDigit:
    case eTreeTypeUnit:
    case eTreeTypeEmpty:
    case eTreeTypeStruct:
    case eTreeTypeUnion:
    case eTreeTypeEnum:
    case eTreeTypeOpaque:
        break;

    case eTreeTypeClosure:
        check_type_recursion(check, tree_fn_get_return(type));
        vector_t *params = tree_fn_get_params(type);
        size_t len = vector_len(params);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *param = vector_get(params, i);
            check_type_recursion(check, param->type);
        }
        break;

    case eTreeTypePointer:
    case eTreeTypeReference:
    case eTreeTypeArray:
        check_type_recursion(check, type->ptr);
        break;

    default: NEVER("invalid type kind `%s` (check-type-size)", tree_to_string(type));
    }
}

static void check_type_recursion(check_t *check, const tree_t *type)
{
    if (set_contains(check->checked_types, type))
    {
        return;
    }

    size_t idx = vector_find(check->type_stack, type);
    if (idx == SIZE_MAX)
    {
        vector_push(&check->type_stack, (tree_t*)type);
        check_inner_type_recursion(check, type);
        vector_drop(check->type_stack);
    }
    else
    {
        event_builder_t id = msg_notify(check->reports, &kEvent_InvalidType, tree_get_node(type),
            "type `%s` contains an impossible type",
            tree_get_name(type)
        );
        size_t len = vector_len(check->type_stack);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *decl = vector_get(check->type_stack, i);
            msg_append(id, tree_get_node(decl), "call to `%s`", tree_get_name(decl));
        }
    }

    set_add(check->checked_types, type);
}

static void check_any_type_recursion(check_t *check, const tree_t *type)
{
    switch (type->kind)
    {
    case eTreeTypeUnion:
    case eTreeTypeStruct:
        check_aggregate_recursion(check, type);
        break;

    default: check_type_recursion(check, type); break;
    }
}

static void check_module_valid(check_t *check, const tree_t *mod)
{
    CTASSERT(check != NULL);
    CTASSERTF(tree_is(mod, eTreeDeclModule), "invalid module `%s`", tree_to_string(mod));

    vector_t *modules = map_values(tree_module_tag(mod, eSemaModules));
    size_t total_modules = vector_len(modules);
    for (size_t i = 0; i < total_modules; i++)
    {
        const tree_t *child = vector_get(modules, i);
        check_module_valid(check, child);
    }

    vector_t *globals = map_values(tree_module_tag(mod, eSemaValues));
    size_t total_globals = vector_len(globals);
    for (size_t i = 0; i < total_globals; i++)
    {
        const tree_t *global = vector_get(globals, i);
        CTASSERTF(tree_is(global, eTreeDeclGlobal), "invalid global `%s`", tree_to_string(global));
        check_simple(check, global);

        check_global_attribs(check, global);
        check_global_recursion(check, global);
    }

    vector_t *functions = map_values(tree_module_tag(mod, eSemaProcs));
    size_t total_functions = vector_len(functions);
    for (size_t i = 0; i < total_functions; i++)
    {
        const tree_t *function = vector_get(functions, i);
        CTASSERTF(tree_is(function, eTreeDeclFunction), "invalid function `%s`", tree_to_string(function));
        check_simple(check, function);

        check_func_attribs(check, function);
        check_func_details(check, function);
    }

    vector_t *types = map_values(tree_module_tag(mod, eSemaTypes));
    size_t total_types = vector_len(types);
    for (size_t i = 0; i < total_types; i++)
    {
        const tree_t *type = vector_get(types, i);
        // check_ident(check, type); TODO: check these properly

        // nothing else can be recursive (TODO: for now)
        check_any_type_recursion(check, type);
    }
}

USE_DECL
void check_tree(logger_t *reports, map_t *mods)
{
    CTASSERT(reports != NULL);
    CTASSERT(mods != NULL);

    arena_t *arena = get_global_arena();
    check_t check = {
        .reports = reports,

        .expr_stack = vector_new(64, arena),
        .type_stack = vector_new(64, arena),

        .checked_exprs = set_new(64, kTypeInfoPtr, arena),
        .checked_types = set_new(64, kTypeInfoPtr, arena),
    };

    map_iter_t iter = map_iter(mods);
    const char *name = NULL;
    tree_t *mod = NULL;
    while (CTU_MAP_NEXT(&iter, &name, &mod))
    {
        // TODO: required checks
        // 0. no nodes are of the wrong type
        // 1. no identifiers are invalid
        // 1. only one 1 entry point of each type is present
        // 2. no types can have infinite size
        // 3. no global types may have circlic dependencies
        // 4. make sure all return statements match the return type of a global or function
        // 5. make sure all operands to nodes are the correct type

        check_module_valid(&check, mod);

        //tree_check(lifetime_get_reports(lifetime), ctx->root);
    }
}
