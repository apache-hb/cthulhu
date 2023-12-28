#include "common.h"

#include "config/config.h"
#include "core/macros.h"
#include "memory/arena.h"
#include "std/typed/vector.h"
#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "io/io.h"

#include "interop/compile.h"

#include "ap_bison.h" // IWYU pragma: keep
#include "ap_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, ap);

static char *join_args(int argc, const char **argv, arena_t *arena)
{
    typevec_t *vec = typevec_new(sizeof(char), argc - 1, arena);
    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        size_t len = strlen(arg);
        typevec_append(vec, arg, len);
    }

    typevec_push(vec, "\0");

    return typevec_data(vec);
}

static int ap_parse_common(ap_t *self, const char *text)
{
    CTASSERT(self != NULL);
    CTASSERT(text != NULL);

    io_t *io = io_string("<command>", text, self->arena);
    scan_t *scan = scan_io("args", io, self->arena);

    scan_set_context(scan, self);
    parse_result_t result = compile_scanner(scan, &kCallbacks);

    return result.result == eParseOk ? EXIT_OK : EXIT_ERROR;
}

int ap_parse_args(ap_t *self, int argc, const char **argv)
{
    char *args = join_args(argc, argv, self->arena);
    return ap_parse_common(self, args);
}

int ap_parse(ap_t *self, const char *str)
{
    return ap_parse_common(self, str);
}

static int get_option_type(const cfg_field_t *field)
{
    switch (cfg_get_type(field))
    {
    case eConfigBool: return AP_BOOL_OPTION;
    case eConfigInt: return AP_INT_OPTION;

    case eConfigEnum:
    case eConfigFlags:
    case eConfigString:
        return AP_STRING_OPTION;

    default:
        NEVER("unknown option type %d", cfg_get_type(field));
    }
}

static const char *get_lookup_name(const char *name, bool *negate, arena_t *arena)
{
    CTASSERT(name != NULL);
    CTASSERT(negate != NULL);

    // if the name ends with `-` then its a negation
    // so we need to strip that off

    *negate = false;
    size_t len = strlen(name);

    // something is wrong if the name is empty
    CTASSERT(len > 0);

    if (name[len - 1] == '-')
    {
        *negate = true;
        len -= 1;
        return arena_strndup(name, len, arena);
    }

    // if the name starts with `no-` then its a negation
    // so we need to strip that off

    if (len > 3 && strncmp(name, "no-", 3) == 0)
    {
        *negate = true;
        return arena_strdup(name + 3, arena);
    }

    return name;
}

int ap_get_opt(ap_t *self, const char *name, ap_field_t *param, char **value)
{
    CTASSERT(self != NULL);
    CTASSERT(name != NULL);
    CTASSERT(param != NULL);

    // if the name ends with `-` then its a negation
    // so we need to strip that off

    bool negate = false;
    const char *lookup = get_lookup_name(name, &negate, self->arena);

    cfg_field_t *result = map_get(self->name_lookup, lookup);
    if (result == NULL)
    {
        *value = arena_strdup(name, self->arena);
        return AP_ERROR;
    }

    ap_field_t out = {
        .field = result,
        .negate = negate
    };

    self->count += 1;
    *param = out;
    cfg_type_t type = get_option_type(result);

    // this is fine
    if (negate && type == eConfigBool)
    {
        return type;
    }

    // user attempting to negate a non-boolean flag
    // TODO: report this as an error

    return type;
}
