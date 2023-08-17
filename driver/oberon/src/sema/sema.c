#include "oberon/sema/decl.h"

#include "cthulhu/mediator/driver.h"

#include "report/report-ext.h"

#include "base/panic.h"

static h2_t *obr_get_decl(h2_t *sema, const size_t *tags, size_t len, const char *name)
{
    for (size_t i = 0; i < len; i++)
    {
        size_t tag = tags[i];
        h2_t *decl = h2_module_get(sema, tag, name);
        if (decl != NULL) { return decl; }
    }
    return NULL;
}

h2_t *obr_get_type(h2_t *sema, const char *name)
{
    const size_t tags[] = { eTagTypes };
    return obr_get_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
}

h2_t *obr_get_module(h2_t *sema, const char *name)
{
    const size_t tags[] = { eTagModules };
    return obr_get_decl(sema, tags, sizeof(tags) / sizeof(size_t), name);
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
    size_t tags[eTagTotal] = {
        [eTagValues] = 32,
        [eTagTypes] = 32,
        [eTagProcs] = 32,
        [eTagModules] = 32,
    };

    gTypeInteger = h2_type_digit(node_builtin(), "INTEGER", eDigitInt, eSignSigned);
    gTypeBoolean = h2_type_bool(node_builtin(), "BOOLEAN");

    h2_t *rt = lifetime_sema_new(lifetime, "oberon", eTagTotal, tags);
    obr_add_decl(rt, eTagTypes, "INTEGER", gTypeInteger);
    obr_add_decl(rt, eTagTypes, "BOOLEAN", gTypeBoolean);

    return rt;
}

vector_t *obr_rt_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "oberon");
    vector_push(&path, "lang");
    return path;
}
