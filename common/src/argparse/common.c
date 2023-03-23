#include "common.h"

#include "cmd-bison.h"

#include "argparse/argparse.h"
#include "report/report.h"

#include "base/util.h"
#include "base/panic.h"

#include "std/map.h"
#include "std/vector.h"

#include <limits.h>
#include <stdio.h>

int arg_parse_opt(argparse_t *argparse, const char *text, param_t **param)
{
    param_t *result = map_get(argparse->params, text);
    if (result == NULL)
    {
        return ERROR;
    }

    *param = result;

    switch (result->kind)
    {
    case eParamBool: return FLAG_OPT;
    case eParamInt: return INT_OPT;
    case eParamString: return STRING_OPT;
    default: return ERROR;
    }
}

void argparse_string_opt(argparse_t *argparse, param_t *param, const char *value)
{
    CTASSERT(argparse != NULL);
    CTASSERT(param != NULL);
    CTASSERT(param->kind == eParamString);

    arg_t *arg = map_get_ptr(argparse->lookup, param);

    if (arg->setByUser)
    {
        report(argparse->reports, eWarn, node_invalid(), "flag `` already set");
    }

    arg->setByUser = true;
    arg->string = value;
}

void argparse_int_opt(argparse_t *argparse, param_t *param, mpz_t value)
{
    CTASSERT(argparse != NULL);
    CTASSERT(param != NULL);
    CTASSERT(param->kind == eParamInt);

    arg_t *arg = map_get_ptr(argparse->lookup, param);

    if (arg->setByUser)
    {
        report(argparse->reports, eWarn, node_invalid(), "flag `` already set");
    }

    arg->setByUser = true;
    arg->digit = mpz_get_si(value);
}

void argparse_flag_opt(argparse_t *argparse, param_t *param)
{
    CTASSERT(argparse != NULL);
    CTASSERT(param != NULL);
    CTASSERT(param->kind == eParamBool);

    arg_t *arg = map_get_ptr(argparse->lookup, param);

    if (arg->setByUser)
    {
        report(argparse->reports, eWarn, node_invalid(), "flag `` already set");
    }

    arg->setByUser = true;
    arg->boolean = true;
}

void argparse_add_file(argparse_t *argparse, char *name)
{
    CTASSERT(argparse != NULL);
    CTASSERT(name != NULL);

    vector_push(&argparse->files, name);
}
