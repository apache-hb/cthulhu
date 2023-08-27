#include "oberon/sema/decl.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/util/util.h"

#include "report/report-ext.h"

#include "base/panic.h"

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

void obr_add_decl(tree_t *sema, obr_tags_t tag, const char *name, tree_t *decl)
{
    const tree_t *old = tree_module_get(sema, tag, name);
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
/// runtime mod
///

static const util_digit_t kBaseDigits[] = {
    { eDigitInt, eSignSigned, "INTEGER" }
};

static const util_config_t kBaseConfig = {
    .langName = "oberon",
    .unitName = "VOID",
    .boolName = "BOOLEAN",
    .stringName = "STRING",

    .digits = kBaseDigits,
    .totalDigits = sizeof(kBaseDigits) / sizeof(util_digit_t),
};

static util_types_t gBaseTypes;

tree_t *obr_get_digit_type(digit_t digit, sign_t sign) { return util_get_digit(gBaseTypes, digit, sign); }
tree_t *obr_get_bool_type(void) { return util_get_bool(gBaseTypes); }

tree_t *obr_rt_mod(lifetime_t *lifetime)
{
    size_t tags[eObrTagTotal] = {
        [eObrTagValues] = 32,
        [eObrTagTypes] = 32,
        [eObrTagProcs] = 32,
        [eObrTagModules] = 32,
    };

    tree_t *rt = lifetime_sema_new(lifetime, "oberon", eObrTagTotal, tags);
    gBaseTypes = util_base_create(kBaseConfig, rt);

    return rt;
}

vector_t *obr_rt_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "oberon");
    vector_push(&path, "lang");
    return path;
}
