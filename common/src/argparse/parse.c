#include "common.h"

#include "std/vector.h"
#include "std/str.h"
#include "std/map.h"

#include "io/io.h"

#include "scan/compile.h"

#include "ap-bison.h"
#include "ap-flex.h"

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

int ap_parse(ap_t *self, reports_t *reports, int argc, const char **argv)
{
    char *args = join_args(argc, argv);
    io_t *io = io_string("<command-line>", args);
    scan_t *scan = scan_io(reports, "ap2", io);

    scan_set(scan, self);
    compile_scanner(scan, &kCallbacks);

    return 0;
}

int ap_get_opt(ap_t *self, const char *name, ap_param_t **param, char **error)
{
    CTASSERT(self != NULL);
    CTASSERT(name != NULL);
    CTASSERT(param != NULL);

    ap_param_t *result = map_get(self->nameLookup, name);
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
