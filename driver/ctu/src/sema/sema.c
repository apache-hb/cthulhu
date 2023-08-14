#include "ctu/sema/sema.h"

#include "cthulhu/hlir/query.h"

#include "cthulhu/mediator/driver.h"

#include "report/report-ext.h"

#include "std/vector.h"

#include "base/panic.h"

///
/// decls
///

static h2_t *get_decl(h2_t *sema, const char *name, const ctu_tag_t *tags, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        h2_t *decl = h2_module_get(sema, tags[i], name);
        if (decl != NULL) { return decl; }
    }

    return NULL;
}

h2_t *ctu_get_namespace(h2_t *sema, const char *name)
{
    ctu_tag_t tags[] = { eTagModules, eTagImports };
    return get_decl(sema, name, tags, sizeof(tags) / sizeof(ctu_tag_t));
}

h2_t *ctu_get_type(h2_t *sema, const char *name)
{
    ctu_tag_t tags[] = { eTagTypes };
    return get_decl(sema, name, tags, sizeof(tags) / sizeof(ctu_tag_t));
}

h2_t *ctu_get_decl(h2_t *sema, const char *name)
{
    ctu_tag_t tags[] = { eTagValues, eTagFunctions };
    return get_decl(sema, name, tags, sizeof(tags) / sizeof(ctu_tag_t));
}

void ctu_add_decl(h2_t *sema, ctu_tag_t tag, const char *name, h2_t *decl)
{
    CTASSERT(name != NULL);
    CTASSERT(decl != NULL);

    h2_t *old = h2_module_get(sema, tag, name);
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
/// runtime
///

static h2_t *kIntTypes[eDigitTotal * eSignTotal] = { NULL };
static h2_t *kBoolType = NULL;

static h2_t *make_int_type(const char *name, digit_t digit, sign_t sign)
{
    return (kIntTypes[digit * eSignTotal + sign] = h2_type_digit(node_builtin(), name, digit, sign));
}

static h2_t *make_bool_type(const char *name)
{
    return (kBoolType = h2_type_bool(node_builtin(), name));
}

h2_t *ctu_get_int_type(digit_t digit, sign_t sign)
{
    return kIntTypes[digit * eSignTotal + sign];
}

h2_t *ctu_get_bool_type(void)
{
    return kBoolType;
}

h2_t *ctu_rt_mod(lifetime_t *lifetime)
{
    GLOBAL_INIT("cthulhu runtime module");

    size_t sizes[eTagTotal] = {
        [eTagValues] = 1,
        [eTagTypes] = 1,
        [eTagFunctions] = 1,
        [eTagModules] = 1,
        [eTagImports] = 1,
        [eTagAttribs] = 1,
        [eTagSuffix] = 1,
    };

    h2_t *root = lifetime_sema_new(lifetime, "runtime", eTagTotal, sizes);

    ctu_add_decl(root, eTagTypes, "bool", make_bool_type("bool"));

    ctu_add_decl(root, eTagTypes, "char", make_int_type("char", eDigitChar, eSignSigned));
    ctu_add_decl(root, eTagTypes, "uchar", make_int_type("uchar", eDigitChar, eSignUnsigned));

    ctu_add_decl(root, eTagTypes, "int", make_int_type("int", eDigitInt, eSignSigned));
    ctu_add_decl(root, eTagTypes, "uint", make_int_type("uint", eDigitInt, eSignUnsigned));

    ctu_add_decl(root, eTagTypes, "long", make_int_type("long", eDigitLong, eSignSigned));
    ctu_add_decl(root, eTagTypes, "ulong", make_int_type("ulong", eDigitLong, eSignUnsigned));

    return root;
}

vector_t *ctu_rt_path(void)
{
    vector_t *path = vector_new(2);
    vector_push(&path, "cthulhu");
    vector_push(&path, "lang");
    return path;
}
