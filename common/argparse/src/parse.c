#include "common.h"

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
    arena_t *arena = ctu_default_alloc();
    io_t *io = io_string("<command-line>", args, arena);
    scan_t *scan = scan_io("ap2", io, ctu_default_alloc());

    scan_set(scan, self);
    parse_result_t result = compile_scanner(scan, &kCallbacks);

    return result.result == eParseOk ? 0 : 1;
}

int ap_get_opt(ap_t *self, const char *name, ap_param_t **param, char **error)
{
    CTASSERT(self != NULL);
    CTASSERT(name != NULL);
    CTASSERT(param != NULL);

    ap_param_t *result = map_get(self->name_lookup, name);
    if (result == NULL)
    {
        *error = format("unknown option '%s'", name);
        return AP_ERROR;
    }

    *param = result;

    switch (result->type)
    {
    case eParamBool: return AP_BOOL;
    case eParamInt: return AP_INT;
    case eParamString: return AP_STRING;
    default: return AP_ERROR;
    }
}
