#include "ctu/sema/sema.h"

#include "cthulhu/tree/tree.h"
#include "cthulhu/tree/query.h"
#include "cthulhu/tree/sema.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/util/util.h"

#include "report/report-ext.h"

#include "std/vector.h"

#include "base/panic.h"
#include "base/util.h"

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

reports_t *ctu_sema_reports(ctu_sema_t sema)
{
    tree_t *it = sema.sema;
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
        report_shadow(sema->reports, name, tree_get_node(old), tree_get_node(decl));
    }
    else
    {
        tree_module_set(sema, tag, name, decl);
    }
}

///
/// extras
///

static const char * const kCurrentLoop = "ctu:current-loop";

tree_t *ctu_current_loop(tree_t *sema)
{
    return tree_get_extra(sema, kCurrentLoop);
}

void ctu_set_current_loop(tree_t *sema, tree_t *loop)
{
    tree_set_extra(sema, kCurrentLoop, loop);
}

///
/// runtime
///

static tree_t *gIntTypes[eDigitTotal * eSignTotal] = { NULL };
static tree_t *gBoolType = NULL;
static tree_t *gVoidType = NULL;
static tree_t *gOpaqueType = NULL;
static tree_t *gStringType = NULL;

static tree_t *gStringChar = NULL;

#define DIGIT_TYPE(DIGIT, SIGN) gIntTypes[(DIGIT) * eSignTotal + (SIGN)]

static tree_t *make_int_type(const char *name, digit_t digit, sign_t sign)
{
    return (DIGIT_TYPE(digit, sign) = tree_type_digit(node_builtin(), name, digit, sign, eQualUnknown));
}

static tree_t *make_bool_type(const char *name)
{
    return (gBoolType = tree_type_bool(node_builtin(), name, eQualUnknown));
}

static tree_t *make_str_type(const char *name)
{
    return (gStringType = tree_type_pointer(node_builtin(), name, gStringChar, SIZE_MAX));
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

tree_t *ctu_get_char_type(void) { return gStringChar; }
tree_t *ctu_get_bool_type(void) { return gBoolType; }
tree_t *ctu_get_void_type(void) { return gVoidType; }

///
/// runtime and builtin modules
///

vector_t *ctu_rt_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "cthulhu");
    vector_push(&path, "lang");
    return path;
}

tree_t *ctu_rt_mod(lifetime_t *lifetime)
{
    GLOBAL_INIT("cthulhu runtime module");

    size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = 1,
        [eCtuTagTypes] = 1,
        [eCtuTagFunctions] = 1,
        [eCtuTagModules] = 1,
        [eCtuTagImports] = 1,
        [eCtuTagAttribs] = 1,
        [eCtuTagSuffixes] = 1,
    };

    gStringChar = tree_type_digit(node_builtin(), "letter", eDigitChar, eSignSigned, eQualConst);

    tree_t *root = lifetime_sema_new(lifetime, "runtime", eCtuTagTotal, sizes);

    ctu_add_decl(root, eCtuTagTypes, "char", make_int_type("char", eDigitChar, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uchar", make_int_type("uchar", eDigitChar, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "short", make_int_type("short", eDigitShort, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "ushort", make_int_type("ushort", eDigitShort, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "int", make_int_type("int", eDigitInt, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "uint", make_int_type("uint", eDigitInt, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "long", make_int_type("long", eDigitLong, eSignSigned));
    ctu_add_decl(root, eCtuTagTypes, "ulong", make_int_type("ulong", eDigitLong, eSignUnsigned));

    ctu_add_decl(root, eCtuTagTypes, "bool", make_bool_type("bool"));
    ctu_add_decl(root, eCtuTagTypes, "str", make_str_type("str"));
    ctu_add_decl(root, eCtuTagTypes, "void", make_void_type("void"));
    ctu_add_decl(root, eCtuTagTypes, "opaque", make_opaque_type("opaque"));

    ctu_init_attribs(root);

    return root;
}
