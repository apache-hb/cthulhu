#include "common.h"

#include "argparse/argparse.h"
#include "report/report.h"

#include "std/map.h"
#include "std/vector.h"

#include <limits.h>

static const char *kFlagTypes[eParamTotal] = {
    [eParamBool] = "boolean",
    [eParamString] = "string",
    [eParamInt] = "integer",
};

static bool argparse_check(argparse_t *argparse, param_kind_t kind)
{
    arg_t *currentArg = argparse->currentArg;
    if (currentArg == NULL)
    {
        return false;
    }

    param_kind_t flagKind = currentArg->kind;
    if (flagKind != kind)
    {
        report(argparse->reports, WARNING, node_invalid(), "flag %s expected a %s, got %s", argparse->currentName,
               kFlagTypes[kind], kFlagTypes[flagKind]);

        argparse->currentArg = NULL;
        argparse->currentName = NULL;

        return false;
    }

    if (currentArg->setByUser)
    {
        report(argparse->reports, WARNING, node_invalid(), "flag `%s` already set", argparse->currentName);
        argparse->currentArg = NULL;
        argparse->currentName = NULL;
        return false;
    }

    return true;
}

static void argparse_flush(argparse_t *argparse)
{
    if (argparse->currentArg != NULL)
    {
        argparse->currentArg->setByUser = true;
    }

    argparse->currentArg = NULL;
    argparse->currentName = NULL;
}

void argparse_end_flag(argparse_t *argparse)
{
    if (argparse_check(argparse, eParamBool))
    {
        argparse->currentArg->boolean = true;
        argparse_flush(argparse);
    }
}

void argparse_begin_flag(argparse_t *argparse, const char *name)
{
    argparse_end_flag(argparse);

    arg_t *arg = map_get(argparse->params, name);
    if (arg == NULL)
    {
        report(argparse->reports, WARNING, node_invalid(), "unknown flag '%s'", name);
        return;
    }

    argparse->currentName = name;
    argparse->currentArg = arg;
}

void argparse_push_string(argparse_t *argparse, const char *value)
{
    if (argparse_check(argparse, eParamString))
    {
        argparse->currentArg->string = value;
        argparse_flush(argparse);
    }
    else
    {
        argparse_end_flag(argparse);
        vector_push(&argparse->files, (char *)value);
    }
}

void argparse_push_digit(argparse_t *argparse, mpz_t value)
{
    if (argparse_check(argparse, eParamInt))
    {
        long digit = mpz_get_si(value);
        if (!mpz_fits_slong_p(value))
        {
            message_t *id = report(argparse->reports, WARNING, node_invalid(),
                                   "flag `%s` passed digit %s, which is out of range [%ld, %ld]", argparse->currentName,
                                   mpz_get_str(NULL, 10, value), LONG_MIN, LONG_MAX);
            report_note(id, "value truncated to %ld", digit);
        }

        argparse->currentArg->digit = digit;
        argparse_flush(argparse);
    }
}
