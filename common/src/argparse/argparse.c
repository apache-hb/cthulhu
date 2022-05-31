#include "argparse/argparse.h"

#include "base/macros.h"
#include "base/util.h"

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

static arg_parse_result_t new_result(const arg_parse_config_t *config)
{
    arg_parse_result_t result = {
        .success = false,
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

arg_parse_result_t arg_parse(const arg_parse_config_t *config)
{
    arg_parse_result_t result = new_result(config);
    
    create_arg_map(result.params, config->args);

    return result;
}
