#include "common.h"

#include "config/config.h"
#include "memory/memory.h"
#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "io/io.h"

#include "interop/compile.h"

#include "ap_bison.h" // IWYU pragma: keep
#include "ap_flex.h" // IWYU pragma: keep

CTU_CALLBACKS(kCallbacks, ap);

static char *join_args(int argc, const char **argv)
{
    vector_t *vec = vector_of(argc - 1);
    for (int i = 1; i < argc; i++)
    {
        vector_set(vec, i - 1, (char *)argv[i]);
    }

    return str_join(" ", vec);
}

int ap_parse(ap_t *self, int argc, const char **argv)
{
    char *args = join_args(argc, argv);
    arena_t *arena = get_global_arena();
    io_t *io = io_string("<command-line>", args, arena);
    scan_t *scan = scan_io("ap2", io, arena);

    scan_set_context(scan, self);
    parse_result_t result = compile_scanner(scan, &kCallbacks);

    return result.result == eParseOk ? 0 : 1;
}

static int get_option_type(const cfg_field_t *field)
{
    switch (cfg_get_type(field))
    {
    case eConfigBool: return AP_BOOL_OPTION;
    case eConfigInt: return AP_INT_OPTION;

    case eConfigEnum:
    case eConfigFlags: // TODO: these need special validation
    case eConfigString:
        return AP_STRING_OPTION;

    default: NEVER("unknown option type %d", cfg_get_type(field));
    }
}

int ap_get_opt(ap_t *self, const char *name, cfg_field_t **param, char **value)
{
    CTASSERT(self != NULL);
    CTASSERT(name != NULL);
    CTASSERT(param != NULL);

    cfg_field_t *result = map_get(self->name_lookup, name);
    if (result == NULL)
    {
        *value = ctu_strdup(name);
        return AP_ERROR;
    }

    *param = result;
    return get_option_type(result);
}
