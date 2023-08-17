#include "oberon/sema/decl.h"

#include "cthulhu/mediator/driver.h"

#include "cthulhu/util/util.h"

#include "report/report-ext.h"

#include "base/panic.h"

h2_t *obr_get_type(h2_t *sema, const char *name)
{
    const size_t tags[] = { eObrTagTypes };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

h2_t *obr_get_module(h2_t *sema, const char *name)
{
    const size_t tags[] = { eObrTagModules };
    return util_select_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

void obr_add_decl(h2_t *sema, obr_tags_t tag, const char *name, h2_t *decl)
{
    const h2_t *old = h2_module_get(sema, tag, name);
    if (old != NULL)
    {
        report_shadow(sema->reports, name, h2_get_node(old), h2_get_node(decl));
    }
    else
    {
        h2_module_set(sema, tag, name, decl);
    }
}

///
/// runtime mod
///

static h2_t *gTypeInteger = NULL;
static h2_t *gTypeBoolean = NULL;

h2_t *obr_get_digit_type(digit_t digit, sign_t sign)
{
    CTASSERT(gTypeInteger != NULL);
    return gTypeInteger;
}

h2_t *obr_get_bool_type(void)
{
    CTASSERT(gTypeBoolean != NULL);
    return gTypeBoolean;
}

h2_t *obr_rt_mod(lifetime_t *lifetime)
{
    size_t tags[eObrTagTotal] = {
        [eObrTagValues] = 32,
        [eObrTagTypes] = 32,
        [eObrTagProcs] = 32,
        [eObrTagModules] = 32,
    };

    gTypeInteger = h2_type_digit(node_builtin(), "INTEGER", eDigitInt, eSignSigned);
    gTypeBoolean = h2_type_bool(node_builtin(), "BOOLEAN");

    h2_t *rt = lifetime_sema_new(lifetime, "oberon", eObrTagTotal, tags);
    obr_add_decl(rt, eObrTagTypes, "INTEGER", gTypeInteger);
    obr_add_decl(rt, eObrTagTypes, "BOOLEAN", gTypeBoolean);

    return rt;
}

vector_t *obr_rt_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "oberon");
    vector_push(&path, "lang");
    return path;
}
