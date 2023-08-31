#include "ctu/sema/sema.h"

#include "cthulhu/tree/query.h"
#include "cthulhu/tree/sema.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/util/util.h"

#include "report/report-ext.h"

#include "std/vector.h"

#include "base/panic.h"
#include "base/util.h"

///
/// decls
///

tree_t *ctu_get_namespace(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagModules, eCtuTagImports };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *ctu_get_type(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagTypes };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

tree_t *ctu_get_decl(tree_t *sema, const char *name)
{
    const size_t tags[] = { eCtuTagValues, eCtuTagFunctions };
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
/// extra data
///

static const char *kCurrentFn = "ctu:current-fn";

void ctu_set_current_fn(tree_t *sema, tree_t *decl)
{
    tree_set_extra(sema, kCurrentFn, decl);
}

tree_t *ctu_get_current_fn(tree_t *sema)
{
    tree_t *decl = tree_get_extra(sema, kCurrentFn);
    CTASSERT(decl != NULL);

    return decl;
}

///
/// runtime
///

static tree_t *kIntTypes[eDigitTotal * eSignTotal] = { NULL };
static tree_t *kBoolType = NULL;
static tree_t *kStrType = NULL;

static tree_t *make_int_type(const char *name, digit_t digit, sign_t sign)
{
    return (kIntTypes[digit * eSignTotal + sign] = tree_type_digit(node_builtin(), name, digit, sign));
}

static tree_t *make_bool_type(const char *name)
{
    return (kBoolType = tree_type_bool(node_builtin(), name));
}

static tree_t *make_str_type(const char *name)
{
    return (kStrType = tree_type_array(node_builtin(), name, ctu_get_int_type(eDigitChar, eSignSigned), SIZE_MAX));
}

tree_t *ctu_get_int_type(digit_t digit, sign_t sign)
{
    return kIntTypes[digit * eSignTotal + sign];
}

tree_t *ctu_get_bool_type(void)
{
    return kBoolType;
}

tree_t *ctu_get_str_type(size_t length)
{
    return tree_type_array(node_builtin(), "str", ctu_get_int_type(eDigitChar, eSignSigned), length);
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

    return root;
}

vector_t *ctu_rt_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "cthulhu");
    vector_push(&path, "lang");
    return path;
}
