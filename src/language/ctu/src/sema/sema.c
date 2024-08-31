// SPDX-License-Identifier: GPL-3.0-only

#include "ctu/sema/sema.h"

#include "cthulhu/broker/broker.h"
#include "cthulhu/events/events.h"
#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "cthulhu/util/util.h"

#include "memory/memory.h"
#include "std/vector.h"

#include "base/panic.h"

///
/// sema
///

ctu_sema_t ctu_sema_init(tree_t *sema, tree_t *decl, vector_t *block)
{
    CTASSERT(sema != NULL);
    CTASSERT(block != NULL);

    ctu_sema_t it = {
        .sema = sema,
        .decl = decl,
        .block = block
    };

    return it;
}

ctu_sema_t ctu_sema_nested(ctu_sema_t *parent, tree_t *sema, tree_t *decl, vector_t *block)
{
    CTASSERT(parent != NULL);
    CTASSERT(decl != NULL);
    CTASSERT(block != NULL);

    ctu_sema_t it = {
        .sema = sema,
        .decl = decl,
        .block = block,
        .current_loop = parent->current_loop,
    };

    return it;
}

ctu_sema_t ctu_sema_child(ctu_sema_t *sema)
{
    CTASSERT(sema != NULL);

    ctu_sema_t it = {
        .sema = sema->sema,
        .decl = sema->decl,
        .block = sema->block,
        .current_loop = sema->current_loop,
    };

    return it;
}

logger_t *ctu_sema_reports(ctu_sema_t *sema)
{
    tree_t *it = sema->sema;
    return it->reports;
}

///
/// decls
///

tree_t *ctu_get_namespace(tree_t *sema, const char *name, bool *imported)
{
    const size_t local[] = { eCtuTagModules };
    search_t search_local = { local, sizeof(local) / sizeof(size_t) };
    void *it = util_select_decl(sema, search_local, name);
    if (it != NULL) { return it; }

    const size_t global[] = { eCtuTagImports };
    search_t search_global = { global, sizeof(global) / sizeof(size_t) };
    it = util_select_decl(sema, search_global, name);
    if (it != NULL)
    {
        if (imported != NULL) { *imported = true; }
        return it;
    }

    return NULL;
}

tree_t *ctu_get_import(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagImports };
    search_t search = { tags, sizeof(tags) / sizeof(size_t) };
    return util_select_decl(sema, search, name);
}

tree_t *ctu_get_type(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagTypes };
    search_t search = { tags, sizeof(tags) / sizeof(size_t) };
    return util_select_decl(sema, search, name);
}

ctu_attrib_t *ctu_get_attrib(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagAttribs };
    search_t search = { tags, sizeof(tags) / sizeof(size_t) };
    return util_select_decl(sema, search, name);
}

tree_t *ctu_get_decl(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagValues, eCtuTagFunctions };
    search_t search = { tags, sizeof(tags) / sizeof(size_t) };
    return util_select_decl(sema, search, name);
}

tree_t *ctu_get_loop(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagLabels };
    search_t search = { tags, sizeof(tags) / sizeof(size_t) };
    return util_select_decl(sema, search, name);
}

void ctu_add_decl(tree_t *sema, ctu_tag_t tag, const char *name, tree_t *decl)
{
    CTASSERT(name != NULL);
    CTASSERT(decl != NULL);

    tree_t *old = tree_module_get(sema, tag, name);
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
/// extras
///

tree_t *ctu_current_loop(ctu_sema_t *sema)
{
    CTASSERT(sema != NULL);

    return sema->current_loop;
}

void ctu_set_current_loop(ctu_sema_t *sema, tree_t *loop)
{
    CTASSERT(sema != NULL);

    sema->current_loop = loop;
}

///
/// runtime
///

static tree_t *gIntTypes[eDigitTotal * eSignTotal] = { NULL };
static tree_t *gBoolType = NULL;
static tree_t *gVoidType = NULL;
static tree_t *gOpaqueType = NULL;
static tree_t *gStringType = NULL;

static tree_t *gLetter = NULL;

#define DIGIT_TYPE(DIGIT, SIGN) gIntTypes[(DIGIT) * eSignTotal + (SIGN)]

static tree_t *make_int_type(const node_t *node, const char *name, digit_t digit, sign_t sign)
{
    return (DIGIT_TYPE(digit, sign) = tree_type_digit(node, name, digit, sign));
}

static tree_t *make_bool_type(const node_t *node, const char *name)
{
    return (gBoolType = tree_type_bool(node, name));
}

static tree_t *make_str_type(const node_t *node, const char *name)
{
    return (gStringType = tree_type_pointer(node, name, gLetter, SIZE_MAX));
}

static tree_t *make_void_type(const node_t *node, const char *name)
{
    return (gVoidType = tree_type_unit(node, name));
}

static tree_t *make_opaque_type(const node_t *node, const char *name)
{
    return (gOpaqueType = tree_type_opaque(node, name));
}

tree_t *ctu_get_int_type(digit_t digit, sign_t sign)
{
    return DIGIT_TYPE(digit, sign);
}

tree_t *ctu_get_char_type(void) { return gLetter; }
tree_t *ctu_get_bool_type(void) { return gBoolType; }
tree_t *ctu_get_void_type(void) { return gVoidType; }

///
/// runtime and builtin modules
///

typedef struct digit_info_t {
    const char *name;
    digit_t digit;
    sign_t sign;
} digit_info_t;

static const digit_info_t kDigitInfo[] = {
    { "char", eDigitChar, eSignSigned },
    { "uchar", eDigitChar, eSignUnsigned },

    { "short", eDigitShort, eSignSigned },
    { "ushort", eDigitShort, eSignUnsigned },

    { "int", eDigitInt, eSignSigned },
    { "uint", eDigitInt, eSignUnsigned },

    { "long", eDigitLong, eSignSigned },
    { "ulong", eDigitLong, eSignUnsigned },

    { "isize", eDigitSize, eSignSigned },
    { "usize", eDigitSize, eSignUnsigned },

    // TODO: better names for these
    { "ptrdiff", eDigitPtr, eSignSigned },
    { "uptrdiff", eDigitPtr, eSignUnsigned },

    { "int8", eDigit8, eSignSigned },
    { "uint8", eDigit8, eSignUnsigned },

    { "int16", eDigit16, eSignSigned },
    { "uint16", eDigit16, eSignUnsigned },

    { "int32", eDigit32, eSignSigned },
    { "uint32", eDigit32, eSignUnsigned },

    { "int64", eDigit64, eSignSigned },
    { "uint64", eDigit64, eSignUnsigned },

    { "intfast8", eDigitFast8, eSignSigned },
    { "uintfast8", eDigitFast8, eSignUnsigned },

    { "intfast16", eDigitFast16, eSignSigned },
    { "uintfast16", eDigitFast16, eSignUnsigned },

    { "intfast32", eDigitFast32, eSignSigned },
    { "uintfast32", eDigitFast32, eSignUnsigned },

    { "intfast64", eDigitFast64, eSignSigned },
    { "uintfast64", eDigitFast64, eSignUnsigned },

    { "intleast8", eDigitLeast8, eSignSigned },
    { "uintleast8", eDigitLeast8, eSignUnsigned },

    { "intleast16", eDigitLeast16, eSignSigned },
    { "uintleast16", eDigitLeast16, eSignUnsigned },

    { "intleast32", eDigitLeast32, eSignSigned },
    { "uintleast32", eDigitLeast32, eSignUnsigned },

    { "intleast64", eDigitLeast64, eSignSigned },
    { "uintleast64", eDigitLeast64, eSignUnsigned },

    { "float", eDigitFloat, eSignDefault },
    { "double", eDigitDouble, eSignDefault },
};

void ctu_rt_mod(language_runtime_t *runtime, tree_t *root)
{
    arena_t *arena = runtime->arena;
    const node_t *node = tree_get_node(root);

    gLetter = tree_type_digit(node, "letter", eDigitChar, eSignSigned);
    tree_set_qualifiers(gLetter, eQualConst);

    for (size_t i = 0; i < sizeof(kDigitInfo) / sizeof(digit_info_t); i++)
    {
        const digit_info_t *info = &kDigitInfo[i];
        tree_t *it = make_int_type(node, info->name, info->digit, info->sign);
        ctu_add_decl(root, eCtuTagTypes, info->name, it);
    }

    ctu_add_decl(root, eCtuTagTypes, "bool", make_bool_type(node, "bool"));
    ctu_add_decl(root, eCtuTagTypes, "str", make_str_type(node, "str"));
    ctu_add_decl(root, eCtuTagTypes, "void", make_void_type(node, "void"));
    ctu_add_decl(root, eCtuTagTypes, "opaque", make_opaque_type(node, "opaque"));

    ctu_init_attribs(root, arena);
}
