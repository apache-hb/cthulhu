#include "argparse/argparse.h"
#include "argparse/common.h"

#include "base/macros.h"
#include "base/util.h"

#include "std/str.h"

#include "scan/compile.h"

#include "cmd-bison.h"
#include "cmd-flex.h"

#include <limits.h>

CT_CALLBACKS(kCallbacks, cmd);

static size_t total_arg_names(vector_t *params)
{
    size_t count = 0;
    for (size_t i = 0; i < vector_len(params); i++)
    {
        param_t *param = vector_get(params, i);
        count += param->totalNames;
    }
    return count;
}

static argparse_t new_argparse(const arg_parse_config_t *config)
{
    argparse_t result = {
        .reports = config->reports,
        .params = map_optimal(total_arg_names(config->args)),
        .extra = vector_new(config->argc - 1)
    };

    return result;
}

static arg_t *new_arg(param_t *param)
{
    arg_t *arg = ctu_malloc(sizeof(arg_t));
    arg->kind = param->kind;
    arg->setByUser = false;
    return arg;
}

static void create_arg_map(map_t *map, vector_t *args)
{
    for (size_t i = 0; i < vector_len(args); i++)
    {
        param_t *param = vector_get(args, i);
        arg_t *arg = new_arg(param);

        for (size_t i = 0; i < param->totalNames; i++)
        {
            map_set(map, param->names[i], arg);
        }
    }
}

long get_digit_arg(const arg_t *arg, long other)
{
    CTASSERT(arg != NULL, "arg is NULL");
    CTASSERT(arg->kind == PARAM_INT, "arg must be an integer");

    if (!arg->setByUser)
    {
        return other;
    }

    // TODO: this shouldnt be a hard error
    CTASSERTF(mpz_fits_slong_p(arg->digit), "arg must be within [%ld, %ld]", LONG_MIN, LONG_MAX);

    return mpz_get_si(arg->digit);
}

const char *get_string_arg(const arg_t *arg, const char *other)
{
    CTASSERT(arg != NULL, "arg is NULL");
    CTASSERT(arg->kind == PARAM_STRING, "arg must be a string");

    if (!arg->setByUser)
    {
        return other;
    }

    return arg->string;
}

bool get_bool_arg(const arg_t *arg, bool other)
{
    CTASSERT(arg != NULL, "arg is NULL");
    CTASSERT(arg->kind == PARAM_BOOL, "arg must be a boolean");

    if (!arg->setByUser)
    {
        return other;
    }

    return arg->boolean;
}

static char *join_args(int argc, const char **argv)
{
    vector_t *vec = vector_of(argc - 1);
    for (int i = 1; i < argc; i++)
    {
        vector_set(vec, i - 1, (char *)argv[i]);
    }
    return str_join(" ", vec);
}

arg_parse_result_t arg_parse(const arg_parse_config_t *config)
{
    char *args = join_args(config->argc, config->argv);

    scan_t scan = scan_string(config->reports, "command-parser", "<command-line>", args);

    report_config_t reportConfig = {
        .limit = 20,
        .warningsAreErrors = false,
    };

    int status = end_reports(config->reports, "command line parsing", reportConfig);
    if (status != 0)
    {
        arg_parse_result_t result = {
            .exitCode = status,
        };

        return result;
    }

    argparse_t argparse = new_argparse(config);
    
    create_arg_map(argparse.params, config->args);

    scan_set(scan, &argparse);
    compile_scanner(scan, &kCallbacks);

    arg_parse_result_t result = {
        .exitCode = end_reports(config->reports, "command line parsing", reportConfig),
        .params = argparse.params,
        .extra = argparse.extra
    };

    return result;
}
