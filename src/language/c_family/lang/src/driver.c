#include "c/driver.h"
#include "c/sema/sema.h"

#include "memory/memory.h"
#include "std/vector.h"

#include "scan/node.h"

#include "cthulhu/runtime/driver.h"

#include "core/macros.h"

static const tree_attribs_t kExportAttribs = {
    .visibility = eVisiblePublic
};

static vector_t *cc_lang_path(arena_t *arena)
{
    vector_t *path = vector_of(2, arena);
    vector_set(path, 0, "cc");
    vector_set(path, 1, "lang");
    return path;
}

static void add_digit(const node_t *node, tree_t *mod, const char *name, digit_t digit, sign_t sign)
{
    tree_t *it = tree_type_digit(node, name, digit, sign);
    tree_set_attrib(it, &kExportAttribs);
    tree_module_set(mod, eCTagTypes, name, it);
}

static tree_t *cc_lang_module(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);
    logger_t *reports = lifetime_get_logger(lifetime);
    tree_cookie_t *cookie = lifetime_get_cookie(lifetime);
    arena_t *arena = lifetime_get_arena(lifetime);

    const node_t *node = handle_get_builtin(handle);
    size_t sizes[eSemaTotal] = {
        [eCTagValues] = 1,
        [eCTagTypes] = 1,
        [eCTagProcs] = 1,
        [eCTagModules] = 1
    };

    tree_t *root = tree_module_root(reports, cookie, node, "runtime", eSemaTotal, sizes, arena);

    // TODO: this is a stopgap until C is properly implemented

    add_digit(node, root, "char", eDigitChar, eSignDefault);
    add_digit(node, root, "charSigned", eDigitChar, eSignSigned);
    add_digit(node, root, "charUnsigned", eDigitChar, eSignUnsigned);

    add_digit(node, root, "short", eDigitShort, eSignDefault);
    add_digit(node, root, "shortSigned", eDigitShort, eSignSigned);
    add_digit(node, root, "shortUnsigned", eDigitShort, eSignUnsigned);

    add_digit(node, root, "int", eDigitInt, eSignDefault);
    add_digit(node, root, "intSigned", eDigitInt, eSignSigned);
    add_digit(node, root, "intUnsigned", eDigitInt, eSignUnsigned);

    add_digit(node, root, "long", eDigitLong, eSignDefault);
    add_digit(node, root, "longSigned", eDigitLong, eSignSigned);
    add_digit(node, root, "longUnsigned", eDigitLong, eSignUnsigned);

    add_digit(node, root, "int8", eDigit8, eSignSigned);
    add_digit(node, root, "uint8", eDigit8, eSignUnsigned);

    add_digit(node, root, "int16", eDigit16, eSignSigned);
    add_digit(node, root, "uint16", eDigit16, eSignUnsigned);

    add_digit(node, root, "int32", eDigit32, eSignSigned);
    add_digit(node, root, "uint32", eDigit32, eSignUnsigned);

    add_digit(node, root, "int64", eDigit64, eSignSigned);
    add_digit(node, root, "uint64", eDigit64, eSignUnsigned);

    return root;
}

void cc_create(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);
    arena_t *arena = lifetime_get_arena(lifetime);

    vector_t *path = cc_lang_path(arena);
    tree_t *root = cc_lang_module(handle);

    context_t *ctx = compiled_new(handle, root);
    add_context(lifetime, path, ctx);
}

void cc_destroy(driver_t *handle)
{
    CT_UNUSED(handle);
}
