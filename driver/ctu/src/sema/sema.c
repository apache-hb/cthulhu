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

static const util_digit_t kBaseDigits[] = {
    { eDigitChar, eSignSigned, "char" },
    { eDigitChar, eSignUnsigned, "uchar" },

    { eDigitShort, eSignSigned, "short" },
    { eDigitShort, eSignUnsigned, "ushort" },

    { eDigitInt, eSignSigned, "int" },
    { eDigitInt, eSignUnsigned, "uint" },

    { eDigitLong, eSignSigned, "long" },
    { eDigitLong, eSignUnsigned, "ulong" },
};

static const util_config_t kBaseConfig = {
    .langName = "cthulhu",
    .boolName = "bool",
    .stringName = "str",

    .digits = kBaseDigits,
    .totalDigits = sizeof(kBaseDigits) / sizeof(util_digit_t)
};

static util_types_t gBaseTypes;

tree_t *ctu_get_int_type(digit_t digit, sign_t sign)
{
    return util_get_digit(gBaseTypes, digit, sign);
}

tree_t *ctu_get_bool_type(void)
{
    return util_get_bool(gBaseTypes);
}

tree_t *ctu_get_str_type(void)
{
    return util_get_string(gBaseTypes);
}

tree_t *ctu_rt_mod(lifetime_t *lifetime)
{
    GLOBAL_INIT("cthulhu runtime module");

    size_t sizes[eCtuTagTotal] = {
        [eCtuTagValues] = 1,
        [eCtuTagTypes] = 16,
        [eCtuTagFunctions] = 1,
        [eCtuTagModules] = 1,
        [eCtuTagImports] = 1,
        [eCtuTagAttribs] = 1,
        [eCtuTagSuffixes] = 1,
    };

    tree_t *root = lifetime_sema_new(lifetime, "runtime", eCtuTagTotal, sizes);

    gBaseTypes = util_base_create(kBaseConfig, root);

    return root;
}

vector_t *ctu_rt_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "cthulhu");
    vector_push(&path, "lang");
    return path;
}
