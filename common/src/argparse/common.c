#include "common.h"

static const char *kFlagTypes[PARAM_TOTAL] = {
    [PARAM_BOOL] = "boolean",
    [PARAM_STRING] = "string",
    [PARAM_INT] = "integer",
};

static bool argparse_check(argparse_t *argparse, param_kind_t kind)
{
    if (argparse->currentFlag == NULL)
    {
        return false;
    }

    param_kind_t flagKind = argparse->currentFlag->kind;
    if (flagKind != kind)
    {
        report(argparse->reports, WARNING, node_invalid(), "flag %s expected a %s, got %s",
            argparse->currentName, kFlagTypes[kind], kFlagTypes[flagKind]);

        argparse->currentFlag = NULL;
        argparse->currentName = NULL;

        return false;
    }

    if (argparse->currentFlag->setByUser)
    {
        report(argparse->reports, WARNING, node_invalid(), "flag `%s` already set", argparse->currentName);
        argparse->currentFlag = NULL;
        argparse->currentName = NULL;
        return false;
    }

    return true;
}

static void argparse_flush(argparse_t *argparse)
{
    if (argparse->currentFlag != NULL)
    {
        argparse->currentFlag->setByUser = true;
    }

    argparse->currentFlag = NULL;
    argparse->currentName = NULL;
}

void argparse_begin_flag(argparse_t *argparse, const char *name)
{
    if (argparse_check(argparse, PARAM_BOOL))
    {
        argparse->currentFlag->boolean = true;
        argparse_flush(argparse);
    }

    arg_t *arg = map_get(argparse->params, name);
    if (arg == NULL)
    {
        report(argparse->reports, WARNING, node_invalid(), "unknown flag '%s'", name);
        return;
    }

    argparse->currentName = name;
    argparse->currentFlag = arg;
}

void argparse_push_string(argparse_t *argparse, const char *value)
{
    if (argparse_check(argparse, PARAM_STRING))
    {
        argparse->currentFlag->string = value;
        argparse_flush(argparse);
    }
    else
    {
        vector_push(&argparse->extra, (char*)value);
    }
}

void argparse_push_digit(argparse_t *argparse, mpz_t value)
{
    if (argparse_check(argparse, PARAM_INT))
    {
        mpz_init_set(argparse->currentFlag->digit, value);
        argparse_flush(argparse);
    }
}
