#include "ctu/sema/sema.h"

#include "cthulhu/events/events.h"
#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"

#include "cthulhu/runtime/driver.h"

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
    void *it = util_select_decl(sema, local, sizeof(local) / sizeof(size_t), name);
    if (it != NULL) { return it; }

    const size_t global[] = { eCtuTagImports };
    it = util_select_decl(sema, global, sizeof(global) / sizeof(size_t), name);
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
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *ctu_get_type(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagTypes };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

ctu_attrib_t *ctu_get_attrib(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagAttribs };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *ctu_get_decl(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagValues, eCtuTagFunctions };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *ctu_get_loop(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagLabels };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
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

static tree_t *make_int_type(const char *name, digit_t digit, sign_t sign)
{
    return (DIGIT_TYPE(digit, sign) = tree_type_digit(node_builtin(), name, digit, sign));
}

static tree_t *make_bool_type(const char *name)
{
    return (gBoolType = tree_type_bool(node_builtin(), name));
}

static tree_t *make_str_type(const char *name)
{
    return (gStringType = tree_type_pointer(node_builtin(), name, gLetter, SIZE_MAX));
}

static tree_t *make_void_type(const char *name)
{
    return (gVoidType = tree_type_unit(node_builtin(), name));
}

static tree_t *make_opaque_type(const char *name)
{
    return (gOpaqueType = tree_type_opaque(node_builtin(), name));
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

vector_t *ctu_rt_path(void)
{
    arena_t *arena = get_global_arena();
    vector_t *path = vector_new(2, arena);
    vector_push(&path, "cthulhu");
    vector_push(&path, "lang");
    return path;
}

tree_t *ctu_rt_mod(lifetime_t *lifetime)
{
    arena_t *arena = lifetime_get_arena(lifetime);

    size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = 1,
        [eCtuTagTypes] = 1,
        [eCtuTagFunctions] = 1,
        [eCtuTagModules] = 1,
        [eCtuTagImports] = 1,
        [eCtuTagAttribs] = 1,
        [eCtuTagSuffixes] = 1,
    };

    gLetter = tree_type_digit(node_builtin(), "letter", eDigitChar, eSignSigned);
    tree_set_qualifiers(gLetter, eQualConst);

    tree_t *root = lifetime_sema_new(lifetime, "runtime", eCtuTagTotal, sizes);

    ctu_add_decl(root, eCtuTagTypes, "char", make_int_type("char", eDigitChar, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uchar", make_int_type("uchar", eDigitChar, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "short", make_int_type("short", eDigitShort, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "ushort", make_int_type("ushort", eDigitShort, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "int", make_int_type("int", eDigitInt, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uint", make_int_type("uint", eDigitInt, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "long", make_int_type("long", eDigitLong, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "ulong", make_int_type("ulong", eDigitLong, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "isize", make_int_type("isize", eDigitSize, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "usize", make_int_type("usize", eDigitSize, eSignUnsigned));

    // simcoe: these should be made into a library

    ctu_add_decl(root, eCtuTagTypes, "int8", make_int_type("int8", eDigit8, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uint8", make_int_type("uint8", eDigit8, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "int16", make_int_type("int16", eDigit16, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uint16", make_int_type("uint16", eDigit16, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "int32", make_int_type("int32", eDigit32, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uint32", make_int_type("uint32", eDigit32, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "int64", make_int_type("int64", eDigit64, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uint64", make_int_type("uint64", eDigit64, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intfast8", make_int_type("intfast8", eDigitFast8, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintfast8", make_int_type("uintfast8", eDigitFast8, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intfast16", make_int_type("intfast16", eDigitFast16, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintfast16", make_int_type("uintfast16", eDigitFast16, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intfast32", make_int_type("intfast32", eDigitFast32, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintfast32", make_int_type("uintfast32", eDigitFast32, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intfast64", make_int_type("intfast64", eDigitFast64, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintfast64", make_int_type("uintfast64", eDigitFast64, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intleast8", make_int_type("intleast8", eDigitLeast8, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintleast8", make_int_type("uintleast8", eDigitLeast8, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intleast16", make_int_type("intleast16", eDigitLeast16, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintleast16", make_int_type("uintleast16", eDigitLeast16, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intleast32", make_int_type("intleast32", eDigitLeast32, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintleast32", make_int_type("uintleast32", eDigitLeast32, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "intleast64", make_int_type("intleast64", eDigitLeast64, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uintleast64", make_int_type("uintleast64", eDigitLeast64, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "float", make_int_type("float", eDigitFloat, eSignDefault));
    ctu_add_decl(root, eCtuTagTypes, "double", make_int_type("double", eDigitDouble, eSignDefault));

    ctu_add_decl(root, eCtuTagTypes, "bool", make_bool_type("bool"));
    ctu_add_decl(root, eCtuTagTypes, "str", make_str_type("str"));
    ctu_add_decl(root, eCtuTagTypes, "void", make_void_type("void"));
    ctu_add_decl(root, eCtuTagTypes, "opaque", make_opaque_type("opaque"));

    ctu_init_attribs(root, arena);

    return root;
}
