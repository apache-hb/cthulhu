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

void cmderror(where_t *where, void *state, scan_t scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), ERROR, node_new(scan, *where), "%s", msg);
}

typedef struct
{
    // general group
    param_t *printHelp;
    param_t *printVersion;

    // reporting group
    param_t *verboseLogging;
    param_t *fatalWarnings;
    param_t *reportLimit;
} argparse_inner_options_t;

static argparse_inner_options_t get_inner_options(argparse_t *argparse)
{
    
}

static group_t *group_general(void)
{
    vector_t *params = vector_new(32);
    ADD_FLAG(params, PARAM_BOOL, "print help message", { "-h", "--help", "-?" });
    ADD_FLAG(params, PARAM_BOOL, "print version", { "-v", "--version" });
    return new_group("general", "general options", params);
}

static group_t *group_reporting(void)
{
    vector_t *params = vector_new(32);
    ADD_FLAG(params, PARAM_BOOL, "enable verbose logging", { "-Wverbose" });
    ADD_FLAG(params, PARAM_BOOL, "all warnings are errors", { "-Werror" });
    ADD_FLAG(params, PARAM_INT, "limit the number of reports printed", { "-Wlimit" });
    return new_group("reporting", "reporting options", params);
}

static size_t total_arg_names(vector_t *groups)
{
    size_t count = 0;
    for (size_t i = 0; i < vector_len(groups); i++)
    {
        group_t *group = vector_get(groups, i);
        count += vector_len(group->params);
    }
    return count;
}

static argparse_t new_argparse(const arg_parse_config_t *config)
{
    argparse_t result = {
        .reports = config->reports,
        .params = map_optimal(total_arg_names(config->groups)),
        .extra = vector_new(config->argc - 1),
        .groups = config->groups,
    };

    return result;
}

static arg_t *new_arg(param_t *param)
{
    CTASSERT(param->generatedArg == NULL, "param already has an associated arg");
    
    // fill in arg info
    arg_t *arg = ctu_malloc(sizeof(arg_t));
    arg->kind = param->kind;
    arg->setByUser = false;
    
    // link the arg to the parameter
    param->generatedArg = arg;

    return arg;
}

static void add_group(map_t *map, group_t *group)
{
    for (size_t i = 0; i < vector_len(group->params); i++)
    {
        param_t *param = vector_get(group->params, i);
        arg_t *arg = new_arg(param);
        for (size_t j = 0; j < param->totalNames; j++)
        {
            printf("arg: %p %s\n", param->names[j], param->names[j]);
            map_set(map, param->names[j], arg);
        }
    }
}

static void add_groups(map_t *map, vector_t *groups)
{
    for (size_t i = 0; i < vector_len(groups); i++)
    {
        group_t *group = vector_get(groups, i);
        add_group(map, group);
    }
}

group_t *new_group(const char *name, const char *desc, vector_t *params)
{
    group_t *group = ctu_malloc(sizeof(group_t));
    group->name = name;
    group->desc = desc;
    group->params = params;
    return group;
}

param_t *new_param(param_kind_t kind, const char *desc, const char **names, size_t total)
{
    param_t *result = ctu_malloc(sizeof(param_t));
    result->kind = kind;
    result->desc = desc;
    result->names = names;
    result->totalNames = total;
    result->generatedArg = NULL;
    return result;
}

long get_digit_arg(const param_t *param, long other)
{
    const arg_t *arg = param->generatedArg;

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

const char *get_string_arg(const param_t *param, const char *other)
{
    const arg_t *arg = param->generatedArg;
    
    CTASSERT(arg != NULL, "arg is NULL");
    CTASSERT(arg->kind == PARAM_STRING, "arg must be a string");

    if (!arg->setByUser)
    {
        return other;
    }

    return arg->string;
}

bool get_bool_arg(const param_t *param, bool other)
{
    const arg_t *arg = param->generatedArg;
    
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

static char *join_names(const param_t *param)
{
    vector_t *vec = vector_of(param->totalNames);
    for (size_t i = 0; i < param->totalNames; i++)
    {
        vector_set(vec, i, (char*)param->names[i]);
    }
    return str_join(", ", vec);
}

static void print_help(const arg_parse_config_t *config)
{
    printf("usage: %s [options & files]\n", config->argv[0]);

    for (size_t i = 0; i < vector_len(config->groups); i++)
    {
        group_t *group = vector_get(config->groups, i);
        printf("%s - %s\n", group->name, group->desc);
        for (size_t j = 0; j < vector_len(group->params); j++)
        {
            param_t *param = vector_get(group->params, j);
            char *names = join_names(param);
            printf("  %-20s : %s\n", names, param->desc);
        }
    }
}

static void print_version(const arg_parse_config_t *config)
{
    printf("version: %u.%u.%u\n", VERSION_MAJOR(config->version), VERSION_MINOR(config->version), VERSION_PATCH(config->version));
}

static int process_args(const arg_parse_config_t *config, argparse_t argparse, report_config_t reportConfig)
{
    int result = end_reports(argparse.reports, "command line parsing", reportConfig);
    if (result != EXIT_OK)
    {
        return result;
    }

    bool printHelp = get_bool_arg(helpArg, false);
    bool printVersion = get_bool_arg(versionArg, false);

    if (printHelp)
    {
        print_help(config);
    }

    if (printVersion)
    {
        print_version(config);
    }

    return EXIT_OK;
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

    add_group(argparse.params, group_general());
    add_group(argparse.params, group_reporting());

    add_groups(argparse.params, config->groups);

    scan_set(scan, &argparse);
    compile_scanner(scan, &kCallbacks);

    int exitCode = process_args(config, argparse, reportConfig);

    arg_parse_result_t result = {
        .exitCode = exitCode,
        .params = argparse.params,
        .extra = argparse.extra
    };

    return result;
}
