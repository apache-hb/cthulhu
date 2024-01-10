#include "c/driver.h"
#include "c/sema/sema.h"

#include "std/vector.h"

#include "scan/node.h"

#include "cthulhu/runtime/driver.h"

#include "core/macros.h"

static const tree_attribs_t kExportAttribs = {
    .visibility = eVisiblePublic
};

static vector_t *cc_lang_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "cc");
    vector_push(&path, "lang");
    return path;
}

static void add_digit(tree_t *mod, const char *name, digit_t digit, sign_t sign)
{
    const node_t *node = node_builtin();
    tree_t *it = tree_type_digit(node, name, digit, sign, eQualUnknown);
    tree_set_attrib(it, &kExportAttribs);
    tree_module_set(mod, eCTagTypes, name, it);
}

static tree_t *cc_lang_module(lifetime_t *lifetime)
{
    logger_t *reports = lifetime_get_logger(lifetime);
    cookie_t *cookie = lifetime_get_cookie(lifetime);

    const node_t *node = node_builtin();
    size_t sizes[eSemaTotal] = {
        [eCTagValues] = 1,
        [eCTagTypes] = 1,
        [eCTagProcs] = 1,
        [eCTagModules] = 1
    };

    tree_t *root = tree_module_root(reports, cookie, node, "runtime", eSemaTotal, sizes);

    // TODO: this is a stopgap until C is properly implemented

    add_digit(root, "char", eDigitChar, eSignDefault);
    add_digit(root, "charSigned", eDigitChar, eSignSigned);
    add_digit(root, "charUnsigned", eDigitChar, eSignUnsigned);

    add_digit(root, "short", eDigitShort, eSignDefault);
    add_digit(root, "shortSigned", eDigitShort, eSignSigned);
    add_digit(root, "shortUnsigned", eDigitShort, eSignUnsigned);

    add_digit(root, "int", eDigitInt, eSignDefault);
    add_digit(root, "intSigned", eDigitInt, eSignSigned);
    add_digit(root, "intUnsigned", eDigitInt, eSignUnsigned);

    add_digit(root, "long", eDigitLong, eSignDefault);
    add_digit(root, "longSigned", eDigitLong, eSignSigned);
    add_digit(root, "longUnsigned", eDigitLong, eSignUnsigned);

    add_digit(root, "int8", eDigit8, eSignSigned);
    add_digit(root, "uint8", eDigit8, eSignUnsigned);

    add_digit(root, "int16", eDigit16, eSignSigned);
    add_digit(root, "uint16", eDigit16, eSignUnsigned);

    add_digit(root, "int32", eDigit32, eSignSigned);
    add_digit(root, "uint32", eDigit32, eSignUnsigned);

    add_digit(root, "int64", eDigit64, eSignSigned);
    add_digit(root, "uint64", eDigit64, eSignUnsigned);

    return root;
}

void cc_create(driver_t *handle)
{
    lifetime_t *lifetime = handle_get_lifetime(handle);

    vector_t *path = cc_lang_path();
    tree_t *root = cc_lang_module(lifetime);

    context_t *ctx = compiled_new(handle, root);
    add_context(lifetime, path, ctx);
}

void cc_destroy(driver_t *handle)
{
    CTU_UNUSED(handle);
}
