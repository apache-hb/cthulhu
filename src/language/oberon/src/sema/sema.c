#include "oberon/sema/sema.h"

#include "cthulhu/events/events.h"
#include "cthulhu/runtime/driver.h"

#include "cthulhu/util/util.h"

#include "base/panic.h"

#include "memory/memory.h"
#include "std/vector.h"

#include "scan/node.h"

tree_t *obr_get_type(tree_t *sema, const char *name)
{
    const size_t tags[] = { eObrTagTypes };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *obr_get_module(tree_t *sema, const char *name)
{
    const size_t tags[] = { eObrTagModules };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *obr_get_symbol(tree_t *sema, obr_tag_t tag, const char *name)
{
    const size_t tags[] = { tag };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *obr_get_namespace(tree_t *sema, const char *name)
{
    const size_t tags[] = { eObrTagModules, eObrTagImports };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

void obr_add_decl(tree_t *sema, obr_tag_t tag, const char *name, tree_t *decl)
{
    const tree_t *old = tree_module_get(sema, tag, name);
    if (old != NULL)
    {
        evt_symbol_shadowed(sema->reports, name, tree_get_node(old), tree_get_node(decl));
    }
    else
    {
        tree_module_set(sema, tag, name, decl);
    }
}

///
/// runtime mod
///

static tree_t *gTypeBoolean = NULL;
static tree_t *gTypeChar = NULL;
static tree_t *gTypeShort = NULL;
static tree_t *gTypeInteger = NULL;
static tree_t *gTypeLong = NULL;
static tree_t *gTypeReal = NULL;
static tree_t *gTypeLongReal = NULL;
static tree_t *gTypeVoid = NULL;

tree_t *obr_get_bool_type(void)
{
    CTASSERT(gTypeBoolean != NULL);
    return gTypeBoolean;
}

tree_t *obr_get_char_type(void)
{
    CTASSERT(gTypeChar != NULL);
    return gTypeChar;
}

tree_t *obr_get_shortint_type(void)
{
    CTASSERT(gTypeShort != NULL);
    return gTypeShort;
}

tree_t *obr_get_integer_type(void)
{
    CTASSERT(gTypeInteger != NULL);
    return gTypeInteger;
}

tree_t *obr_get_string_type(const node_t *node, size_t length)
{
    CTASSERT(gTypeChar != NULL);
    return tree_type_array(node, "STRING", gTypeChar, length + 1);
}

tree_t *obr_get_longint_type(void)
{
    CTASSERT(gTypeLong != NULL);
    return gTypeLong;
}

tree_t *obr_get_real_type(void)
{
    CTASSERT(gTypeReal != NULL);
    return gTypeReal;
}

tree_t *obr_get_longreal_type(void)
{
    CTASSERT(gTypeLongReal != NULL);
    return gTypeLongReal;
}

tree_t *obr_get_void_type(void)
{
    CTASSERT(gTypeVoid != NULL);
    return gTypeVoid;
}

tree_t *obr_rt_mod(lifetime_t *lifetime)
{
    size_t tags[eObrTagTotal] = {
        [eObrTagValues] = 32,
        [eObrTagTypes] = 32,
        [eObrTagProcs] = 32,
        [eObrTagModules] = 32,
    };

    gTypeBoolean = tree_type_bool(node_builtin(), "BOOLEAN");
    gTypeChar = tree_type_digit(node_builtin(), "CHAR", eDigitChar, eSignSigned);
    gTypeShort = tree_type_digit(node_builtin(), "SHORTINT", eDigitShort, eSignSigned);
    gTypeInteger = tree_type_digit(node_builtin(), "INTEGER", eDigitInt, eSignSigned);
    gTypeLong = tree_type_digit(node_builtin(), "LONGINT", eDigitLong, eSignSigned);
    gTypeVoid = tree_type_unit(node_builtin(), "VOID");

    tree_t *rt = lifetime_sema_new(lifetime, "oberon", eObrTagTotal, tags);
    obr_add_decl(rt, eObrTagTypes, "BOOLEAN", gTypeBoolean);
    obr_add_decl(rt, eObrTagTypes, "CHAR", gTypeChar);
    obr_add_decl(rt, eObrTagTypes, "SHORTINT", gTypeShort);
    obr_add_decl(rt, eObrTagTypes, "INTEGER", gTypeInteger);
    obr_add_decl(rt, eObrTagTypes, "LONGINT", gTypeLong);
    obr_add_decl(rt, eObrTagTypes, "VOID", gTypeVoid);

    return rt;
}

vector_t *obr_rt_path(void)
{
    arena_t *arena = get_global_arena();
    vector_t *path = vector_new(2, arena);
    vector_push(&path, "oberon");
    vector_push(&path, "lang");
    return path;
}
