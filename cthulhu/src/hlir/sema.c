#include "cthulhu/hlir/sema.h"
#include "base/macros.h"
#include "base/memory.h"
#include "base/panic.h"
#include "base/util.h"

#include "cthulhu/hlir/attribs.h"
#include "cthulhu/hlir/decl.h"
#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/query.h"
#include "report/report.h"

#include <string.h>

typedef struct sema_t
{
    struct sema_t *parent;
    reports_t *reports;

    /**
     * an array of maps
     * each map is its own namespace which maps from symbol name to hlir node
     */
    vector_t *decls;

    void *data;
} sema_t;

sema_t *sema_new(sema_t *parent, reports_t *reports, size_t decls, size_t *sizes)
{
    reports_t *innerReports = parent != NULL ? parent->reports : reports;

    CTASSERT(innerReports != NULL);

    sema_t *sema = ctu_malloc(sizeof(sema_t));

    sema->parent = parent;
    sema->reports = innerReports;

    sema->decls = vector_of(decls);
    for (size_t i = 0; i < decls; i++)
    {
        map_t *map = map_optimal(sizes[i]);
        vector_set(sema->decls, i, map);
    }

    return sema;
}

reports_t *sema_reports(sema_t *sema)
{
    CTASSERT(sema != NULL);
    return sema->reports;
}

sema_t *sema_parent(sema_t *sema)
{
    CTASSERT(sema != NULL);
    return sema->parent;
}

void sema_delete(sema_t *sema)
{
    ctu_free(sema);
}

void sema_set_data(sema_t *sema, void *data)
{
    sema->data = data;
}

void *sema_get_data(sema_t *sema)
{
    return sema->data;
}

void sema_set(sema_t *sema, size_t tag, const char *name, void *data)
{
    map_t *map = sema_tag(sema, tag);
    map_set(map, name, data);
}

void *sema_get(sema_t *sema, size_t tag, const char *name)
{
    map_t *map = sema_tag(sema, tag);

    hlir_t *hlir = map_get(map, name);
    if (hlir != NULL)
    {
        return hlir;
    }

    if (sema->parent != NULL)
    {
        return sema_get(sema->parent, tag, name);
    }

    return NULL;
}

map_t *sema_tag(sema_t *sema, size_t tag)
{
    return vector_get(sema->decls, tag);
}

static void report_recursion(reports_t *reports, vector_t *stack, const char *msg)
{
    hlir_t *top = vector_tail(stack);
    node_t *topNode = get_hlir_node(top);
    message_t *id = report(reports, eFatal, topNode, "%s", msg);

    for (size_t i = 0; i < vector_len(stack); i++)
    {
        hlir_t *hlir = vector_get(stack, i);
        node_t *node = get_hlir_node(hlir);
        report_append(id, node, "trace `%zu`", i);
    }
}

static bool find_recursion(reports_t *reports, vector_t **vec, const hlir_t *hlir, const char *msg)
{
    vector_t *stack = *vec;
    for (size_t i = 0; i < vector_len(stack); i++)
    {
        hlir_t *item = vector_get(stack, i);
        if (item != hlir)
        {
            continue;
        }

        report_recursion(reports, stack, msg);
        return true;
    }

    return false;
}

static void check_recursion(reports_t *reports, vector_t **stack, const hlir_t *hlir)
{
    if (hlir == NULL)
    {
        return;
    }
    if (find_recursion(reports, stack, hlir, "recursive variable computation"))
    {
        return;
    }

    vector_push(stack, (hlir_t *)hlir);

    switch (hlir->type)
    {
    case eHlirLoad:
        check_recursion(reports, stack, hlir->read);
        break;
    case eHlirGlobal:
    case eHlirLocal:
        check_recursion(reports, stack, hlir->value);
        break;
    case eHlirBinary:
    case eHlirCompare:
        check_recursion(reports, stack, hlir->lhs);
        check_recursion(reports, stack, hlir->rhs);
        break;

    case eHlirUnary:
        check_recursion(reports, stack, hlir->operand);
        break;

    case eHlirDigitLiteral:
    case eHlirBoolLiteral:
    case eHlirStringLiteral:
        break;

    default:
        ctu_assert(reports, "check-recursion unexpected hlir type %s", hlir_kind_to_string(get_hlir_kind(hlir)));
        break;
    }

    vector_drop(*stack);
}

typedef struct
{
    const hlir_t *hlir;
    bool nesting;
} entry_t;

static entry_t *entry_new(const hlir_t *hlir, bool nesting)
{
    entry_t *entry = ctu_malloc(sizeof(entry_t));
    entry->hlir = hlir;
    entry->nesting = nesting;
    return entry;
}

static void report_type_recursion(reports_t *reports, vector_t *stack)
{
    entry_t *top = vector_tail(stack);
    node_t *topNode = get_hlir_node(top->hlir);
    message_t *id = report(reports, eFatal, topNode, "%s", "recursive type definition");

    size_t trace = 0;

    for (size_t i = 0; i < vector_len(stack); i++)
    {
        entry_t *entry = vector_get(stack, i);
        if (entry == NULL)
        {
            continue;
        }

        node_t *node = get_hlir_node(entry->hlir);
        report_append(id, node, "trace `%zu`", trace++);
    }
}

static bool find_type_recursion(reports_t *reports, vector_t **vec, const hlir_t *hlir, bool nesting, bool opaque)
{
    vector_t *stack = *vec;
    for (size_t i = 0; i < vector_len(stack); i++)
    {
        entry_t *item = vector_get(stack, i);
        if (item == NULL)
        {
            continue;
        }

        if (item->hlir == hlir)
        {
            if (item->nesting && opaque)
            {
                break;
            }

            report_type_recursion(reports, stack);

            vector_push(vec, NULL); // TODO: find a prettier way of keeping the stack intact
            return false;
        }
    }

    vector_push(vec, entry_new(hlir, nesting));

    return true;
}

#define DEPTH_LIMIT 128 // this is what a release deadline looks like :^)

static const hlir_t *chase(reports_t *reports, const hlir_t *hlir)
{
    size_t depth = 0;

    while (true)
    {
        switch (get_hlir_kind(hlir))
        {
        case eHlirPointer:
            hlir = hlir->ptr;
            break;
        case eHlirAlias:
            hlir = hlir->alias;
            break;
        case eHlirRecordField:
            hlir = get_hlir_type(hlir);
            break;
        default:
            return hlir;
        }

        if (depth++ > DEPTH_LIMIT)
        {
            node_t *node = get_hlir_node(hlir);
            message_t *id = report(reports, eFatal, node, "type definition recurses too deep");
            report_note(id, "type definition recurses beyond %d levels", DEPTH_LIMIT);
            return NULL;
        }
    }
}

static void check_type_recursion(reports_t *reports, vector_t **stack, const hlir_t *hlir)
{
    if (hlir == NULL)
    {
        return;
    }

    hlir_kind_t kind = get_hlir_kind(hlir);
    bool result = true;
    switch (kind)
    {
    case eHlirArray:
        result = find_type_recursion(reports, stack, chase(reports, hlir->element), false, true);
        break;

    case eHlirPointer:
        result = find_type_recursion(reports, stack, chase(reports, hlir), false, true);
        break;

    case eHlirClosure:
    case eHlirFunction:
        if ((result = find_type_recursion(reports, stack, hlir, false, true)))
        {
            check_type_recursion(reports, stack, hlir->result);
            for (size_t i = 0; i < vector_len(hlir->params); i++)
            {
                hlir_t *param = vector_get(hlir->params, i);
                check_type_recursion(reports, stack, param);
            }
        }
        break;

    case eHlirStruct:
    case eHlirUnion:
        if ((result = find_type_recursion(reports, stack, hlir, true, false)))
        {
            for (size_t i = 0; i < vector_len(hlir->fields); i++)
            {
                hlir_t *field = vector_get(hlir->fields, i);
                check_type_recursion(reports, stack, field);
            }
        }
        break;

    case eHlirAlias:
        if (find_type_recursion(reports, stack, hlir, false, false))
        {
            check_type_recursion(reports, stack, hlir->alias);
        }
        break;

    case eHlirRecordField:
        check_type_recursion(reports, stack, get_hlir_type(hlir));
        return;

    case eHlirDigit:
    case eHlirBool:
    case eHlirString:
    case eHlirEmpty:
    case eHlirUnit:
        return; // it is important to return rather than break
                // in cases where find_type_recursion isnt called

    default:
        ctu_assert(reports, "check-type-recursion unexpected hlir type %s", hlir_kind_to_string(get_hlir_kind(hlir)));
        return;
    }

    vector_drop(*stack);
}

static bool can_mangle_name(hlir_linkage_t linkage)
{
    switch (linkage)
    {
    case eLinkInternal: // it makes no sense to mangle internal symbols
    case eLinkEntryGui: // these are defined by the platform and cannot be mangled
    case eLinkEntryCli:
        return false;

    default: // everything else is fair game
        return true;
    }
}

static const char *kLinkageNames[eLinkTotal] = {
    [eLinkInternal] = "a symbol with internal linkage",
    [eLinkEntryGui] = "the gui entry point",
    [eLinkEntryCli] = "the cli entry point",
};

static void report_multiple_entry(check_t *ctx, const hlir_t *hlir, const hlir_t *prev, const char *name)
{
    node_t *newNode = get_hlir_node(hlir);
    node_t *prevNode = get_hlir_node(prev);

    message_t *id = report(ctx->reports, eFatal, newNode, "multiple %s entry points defined", name);
    report_append(id, prevNode, "previously defined here");
}

static void check_attribute(check_t *ctx, hlir_t *hlir)
{
    const hlir_attributes_t *attribs = get_hlir_attributes(hlir);

    if (attribs->mangle != NULL && !can_mangle_name(attribs->linkage))
    {
        node_t *node = get_hlir_node(hlir);
        message_t *id =
            report(ctx->reports, eWarn, node, "cannot change name mangling of %s", kLinkageNames[attribs->linkage]);
        report_note(id, "attribute will not be mangled");

        hlir_attributes_t *newAttribs = ctu_memdup(attribs, sizeof(hlir_attributes_t));
        newAttribs->mangle = NULL;
        hlir_set_attributes(hlir, newAttribs);
    }

    if (attribs->linkage == eLinkImported && hlir_is(hlir, eHlirFunction) && hlir->body != NULL)
    {
        report(ctx->reports, eWarn, get_hlir_node(hlir), "imported function with a body, body will be discarded");
    }

    bool isEntry = false;

    if (attribs->linkage == eLinkEntryCli)
    {
        if (ctx->cliEntryPoint != NULL)
        {
            report_multiple_entry(ctx, hlir, ctx->cliEntryPoint, "cli");
        }

        ctx->cliEntryPoint = hlir;
        isEntry = true;
    }

    if (attribs->linkage == eLinkEntryGui)
    {
        if (ctx->guiEntryPoint != NULL)
        {
            report_multiple_entry(ctx, hlir, ctx->guiEntryPoint, "gui");
        }

        ctx->guiEntryPoint = hlir;
        isEntry = true;
    }

    if (isEntry && !hlir_is(hlir, eHlirFunction))
    {
        report(ctx->reports, eFatal, get_hlir_node(hlir), "only functions can be declared as entry points");
    }
}

static void check_identifier_isnt_empty(reports_t *reports, const hlir_t *ident)
{
    const char *name = get_hlir_name(ident);
    if (name == NULL || strlen(name) == 0)
    {
        node_t *node = get_hlir_node(ident);
        report(reports, eFatal, node, "empty identifier");
    }
}

void check_module(check_t *check, hlir_t *mod)
{
    reports_t *reports = check->reports;

    size_t totalGlobals = vector_len(mod->globals);
    size_t totalTypes = vector_len(mod->types);
    size_t totalFunctions = vector_len(mod->functions);
    vector_t *recursionStack = vector_new(16);

    for (size_t i = 0; i < totalTypes; i++)
    {
        hlir_t *type = vector_get(mod->types, i);

        check_identifier_isnt_empty(reports, type);
        check_type_recursion(reports, &recursionStack, type);

        vector_reset(recursionStack);
    }

    for (size_t i = 0; i < totalGlobals; i++)
    {
        hlir_t *var = vector_get(mod->globals, i);
        check_identifier_isnt_empty(reports, var);
        CTASSERTF(hlir_is(var, eHlirGlobal), "check-module polluted: global `%s` is %s, not global", get_hlir_name(var),
                  hlir_kind_to_string(get_hlir_kind(var)));
        check_recursion(reports, &recursionStack, var);

        vector_reset(recursionStack);
    }

#define CHECK_ATTRIBUTES_FOR_ARRAY(length, vector)                                                                     \
    do                                                                                                                 \
    {                                                                                                                  \
        for (size_t i = 0; i < (length); i++)                                                                          \
        {                                                                                                              \
            hlir_t *var = vector_get(vector, i);                                                                       \
            check_attribute(check, var);                                                                               \
            check_identifier_isnt_empty(reports, var);                                                                 \
        }                                                                                                              \
    } while (0)

    CHECK_ATTRIBUTES_FOR_ARRAY(totalGlobals, mod->globals);
    CHECK_ATTRIBUTES_FOR_ARRAY(totalTypes, mod->types);
    CHECK_ATTRIBUTES_FOR_ARRAY(totalFunctions, mod->functions);
}
