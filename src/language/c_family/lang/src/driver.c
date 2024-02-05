#include "c/driver.h"
#include "c/sema/sema.h"

#include "memory/memory.h"
#include "std/vector.h"

#include "scan/node.h"

#include "cthulhu/broker/broker.h"

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

static void cc_lang_module(tree_t *root)
{
    // TODO: this is a stopgap until C is properly implemented
    const node_t *node = tree_get_node(root);

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
}

void cc_create(language_runtime_t *runtime, tree_t *root)
{
    CT_UNUSED(runtime);

    cc_lang_module(root);
}

void cc_destroy(language_runtime_t *runtime)
{
    CT_UNUSED(runtime);
}
