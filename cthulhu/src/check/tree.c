#include "common.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/util/util.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "report/report.h"

#include "std/vector.h"
#include "std/set.h"
#include "std/map.h"
#include "std/str.h"

#include "base/panic.h"

typedef struct check_t {
    reports_t *reports;

    const tree_t *cliEntryPoint;
    const tree_t *guiEntryPoint;

    vector_t *exprStack;
    vector_t *typeStack;

    set_t *checkedExprs;
    set_t *checkedTypes;
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
    const attribs_t *attribs = tree_get_attrib(global);
    switch (attribs->link)
    {
    case eLinkImport:
        if (global->initial != NULL)
        {
            message_t *id = report(check->reports, eWarn, tree_get_node(global),
                "global `%s` is marked as imported but has an implementation",
                tree_get_name(global)
            );
            report_note(id, "implementation will be ignored");
        }
        break;

    case eLinkModule:
        if (attribs->mangle != NULL)
        {
            message_t *id = report(check->reports, eWarn, tree_get_node(global),
                "global `%s` has internal linkage and user defined mangling",
                tree_get_name(global)
            );
            report_note(id, "attribute will be ignored");
        }
        break;

    case eLinkEntryGui:
    case eLinkEntryCli:
        report(check->reports, eFatal, tree_get_node(global),
            "global `%s` is marked as an entry point but is not a function",
            tree_get_name(global)
        );
        break;

    default: break;
    }
}

static void check_func_attribs(check_t *check, const tree_t *fn)
{
    const attribs_t *attribs = tree_get_attrib(fn);

    switch (attribs->link)
    {
    case eLinkImport:
        if (fn->body != NULL)
        {
            message_t *id = report(check->reports, eWarn, tree_get_node(fn),
                "function `%s` is marked as imported but has an implementation",
                tree_get_name(fn)
            );
            report_note(id, "implementation will be ignored");
        }
        break;

    case eLinkModule:
        if (attribs->mangle != NULL)
        {
            message_t *id = report(check->reports, eWarn, tree_get_node(fn),
                "function `%s` has internal linkage and user defined mangling",
                tree_get_name(fn)
            );
            report_note(id, "attribute will be ignored");
        }
        break;

    case eLinkEntryCli:
        if (check->cliEntryPoint != NULL)
        {
            message_t *id = report(check->reports, eFatal, tree_get_node(fn),
                "multiple CLI entry points defined"
            );
            report_append(id, tree_get_node(check->cliEntryPoint), "previous entry point defined here");
        }
        else
        {
            check->cliEntryPoint = fn;
        }
        break;
    case eLinkEntryGui:
        if (check->guiEntryPoint != NULL)
        {
            message_t *id = report(check->reports, eFatal, tree_get_node(fn),
                "multiple GUI entry points defined"
            );
            report_append(id, tree_get_node(check->guiEntryPoint), "previous entry point defined here");
        }
        else
        {
            check->guiEntryPoint = fn;
        }
        break;

    default: break;
    }
}

static void check_func_return_equal(check_t *check, const tree_t *returnType, const tree_t *realType)
{
    if (util_types_equal(returnType, realType)) { return; }

    report(check->reports, eFatal, tree_get_node(realType),
        "return type `%s` does not match function return type `%s`",
        tree_to_string(realType),
        tree_to_string(returnType)
    );
}

static void check_func_body(check_t *check, const tree_t *returnType, const tree_t *stmt)
{
    switch (stmt->kind)
    {
    case eTreeStmtBlock:
        for (size_t i = 0; i < vector_len(stmt->stmts); i++)
        {
            check_func_body(check, returnType, vector_get(stmt->stmts, i));
        }
        break;

    case eTreeStmtLoop:
    case eTreeStmtBranch:
    case eTreeStmtAssign:
        break;

    case eTreeStmtReturn:
        check_func_return_equal(check, returnType, tree_get_type(stmt->value));
        break;

    case eTreeExprCompare:
    case eTreeExprBinary:
    case eTreeExprLoad:
    case eTreeExprCall:
        break; // TODO: check

    default:
        NEVER("invalid statement kind %s (check-func-body)", tree_to_string(stmt));
    }
}

static bool will_always_return(const tree_t *stmt)
{
    switch (stmt->kind)
    {
    case eTreeStmtReturn:
        return true;
    case eTreeStmtBranch:
        return will_always_return(stmt->then) && will_always_return(stmt->other);
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

static void check_func_return(check_t *check, const tree_t *fn)
{
    if (fn->body == NULL) { return; }

    const tree_t *fnType = tree_get_type(fn);
    const tree_t *returnType = fnType->result;

    check_func_body(check, returnType, fn->body);

    if (!will_always_return(fn->body))
    {
        if (tree_is(returnType, eTreeTypeUnit)) { return; }

        report(check->reports, eFatal, tree_get_node(fn),
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
    if (set_contains_ptr(check->checkedExprs, global))
    {
        return;
    }

    size_t idx = vector_find(check->exprStack, global);
    if (idx == SIZE_MAX)
    {
        if (global->initial != NULL)
        {
            vector_push(&check->exprStack, (tree_t*)global);
            check_expr_recursion(check, global->initial);
            vector_drop(check->exprStack);
        }
    }
    else
    {
        message_t *id = report(check->reports, eFatal, tree_get_node(global),
            "evaluation of `%s` may be infinite",
            tree_get_name(global)
        );
        size_t len = vector_len(check->exprStack);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *decl = vector_get(check->exprStack, i);
            report_append(id, tree_get_node(decl), "call to `%s`", tree_get_name(decl));
        }
    }

    set_add_ptr(check->checkedExprs, global);
}

///
/// recursive struct checking
///

static void check_struct_recursion(check_t *check, const tree_t *type);

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
        break;

    case eTreeTypeStruct:
        check_struct_recursion(check, type);
        break;

    default: NEVER("invalid type kind %s (check-type-size)", tree_to_string(type));
    }
}

static void check_struct_recursion(check_t *check, const tree_t *type)
{
    if (set_contains_ptr(check->checkedTypes, type))
    {
        return;
    }

    size_t idx = vector_find(check->typeStack, type);
    if (idx == SIZE_MAX)
    {
        vector_push(&check->typeStack, (tree_t*)type);
        size_t len = vector_len(type->fields);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *field = vector_get(type->fields, i);
            check_struct_type_recursion(check, field->type);
        }
        vector_drop(check->typeStack);
    }
    else
    {
        message_t *id = report(check->reports, eFatal, tree_get_node(type),
            "size of type `%s` may be infinite",
            tree_get_name(type)
        );
        size_t len = vector_len(check->typeStack);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *decl = vector_get(check->typeStack, i);
            report_append(id, tree_get_node(decl), "call to `%s`", tree_get_name(decl));
        }
    }

    set_add_ptr(check->checkedTypes, type);
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
    if (set_contains_ptr(check->checkedTypes, type))
    {
        return;
    }

    size_t idx = vector_find(check->typeStack, type);
    if (idx == SIZE_MAX)
    {
        vector_push(&check->typeStack, (tree_t*)type);
        check_inner_type_recursion(check, type);
        vector_drop(check->typeStack);
    }
    else
    {
        message_t *id = report(check->reports, eFatal, tree_get_node(type),
            "type `%s` contains an impossible type",
            tree_get_name(type)
        );
        size_t len = vector_len(check->typeStack);
        for (size_t i = 0; i < len; i++)
        {
            const tree_t *decl = vector_get(check->typeStack, i);
            report_append(id, tree_get_node(decl), "call to `%s`", tree_get_name(decl));
        }
    }

    set_add_ptr(check->checkedTypes, type);
}

static void check_any_type_recursion(check_t *check, const tree_t *type)
{
    switch (type->kind)
    {
    case eTreeTypeStruct: check_struct_recursion(check, type); break;

    default: check_type_recursion(check, type); break;
    }
}

static void check_module_valid(check_t *check, const tree_t *mod)
{
    CTASSERT(check != NULL);
    CTASSERTF(tree_is(mod, eTreeDeclModule), "invalid module `%s`", tree_to_string(mod));

    logverbose("check %s", tree_get_name(mod));

    vector_t *modules = map_values(tree_module_tag(mod, eSemaModules));
    size_t totalModules = vector_len(modules);
    for (size_t i = 0; i < totalModules; i++)
    {
        const tree_t *child = vector_get(modules, i);
        check_module_valid(check, child);
    }

    vector_t *globals = map_values(tree_module_tag(mod, eSemaValues));
    size_t totalGlobals = vector_len(globals);
    for (size_t i = 0; i < totalGlobals; i++)
    {
        const tree_t *global = vector_get(globals, i);
        CTASSERTF(tree_is(global, eTreeDeclGlobal), "invalid global `%s`", tree_to_string(global));
        check_simple(check, global);

        check_global_attribs(check, global);
        check_global_recursion(check, global);
    }

    vector_t *functions = map_values(tree_module_tag(mod, eSemaProcs));
    size_t totalFunctions = vector_len(functions);
    for (size_t i = 0; i < totalFunctions; i++)
    {
        const tree_t *function = vector_get(functions, i);
        CTASSERTF(tree_is(function, eTreeDeclFunction), "invalid function `%s`", tree_to_string(function));
        check_simple(check, function);

        check_func_attribs(check, function);
        check_func_return(check, function);
    }

    vector_t *types = map_values(tree_module_tag(mod, eSemaTypes));
    size_t totalTypes = vector_len(types);
    for (size_t i = 0; i < totalTypes; i++)
    {
        const tree_t *type = vector_get(types, i);
        // check_ident(check, type); TODO: check these properly

        // nothing else can be recursive (TODO: for now)
        check_any_type_recursion(check, type);
    }
}

void check_tree(reports_t *reports, map_t *mods)
{
    check_t check = {
        .reports = reports,

        .exprStack = vector_new(64),
        .typeStack = vector_new(64),

        .checkedExprs = set_new(64),
        .checkedTypes = set_new(64)
    };

    map_iter_t iter = map_iter(mods);
    while (map_has_next(&iter))
    {
        map_entry_t entry = map_next(&iter);
        tree_t *mod = entry.value;

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
