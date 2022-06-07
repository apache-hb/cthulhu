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
} argparse_state_t;

static const char *kHelpArgs[] = { "-h", "--help", "-?" };
#define TOTAL_HELP_ARGS (sizeof(kHelpArgs) / sizeof(const char *))

static const char *kVersionArgs[] = { "-v", "--version" };
#define TOTAL_VERSION_ARGS (sizeof(kVersionArgs) / sizeof(const char *))

static const char *kVerboseLoggingArgs[] = { "-Wverbose" };
#define TOTAL_VERBOSE_LOGGING_ARGS (sizeof(kVerboseLoggingArgs) / sizeof(const char *))

static const char *kFatalWarningsArgs[] = { "-Werror" };
#define TOTAL_FATAL_WARNINGS_ARGS (sizeof(kFatalWarningsArgs) / sizeof(const char *))

static const char *kReportLimitArgs[] = { "-Wlimit" };
#define TOTAL_REPORT_LIMIT_ARGS (sizeof(kReportLimitArgs) / sizeof(const char *))

static argparse_state_t state_new(void)
{
    argparse_state_t state = {
        .printHelp = new_param(PARAM_BOOL, "print this help message", kHelpArgs, TOTAL_HELP_ARGS),
        .printVersion = new_param(PARAM_BOOL, "print the version number", kVersionArgs, TOTAL_VERSION_ARGS),

        .verboseLogging = new_param(PARAM_BOOL, "enable verbose logging", kVerboseLoggingArgs, TOTAL_VERBOSE_LOGGING_ARGS),
        .fatalWarnings = new_param(PARAM_BOOL, "enable fatal warnings", kFatalWarningsArgs, TOTAL_FATAL_WARNINGS_ARGS),
        .reportLimit = new_param(PARAM_INT, "set the report limit", kReportLimitArgs, TOTAL_REPORT_LIMIT_ARGS),
    };

    return state;
}

static group_t *group_general(const argparse_state_t *state)
{
    vector_t *params = vector_new(4);
    vector_push(&params, state->printHelp);
    vector_push(&params, state->printVersion);
    return new_group("general", "general options", params);
}

static group_t *group_reporting(const argparse_state_t *state)
{
    vector_t *params = vector_new(4);
    vector_push(&params, state->verboseLogging);
    vector_push(&params, state->fatalWarnings);
    vector_push(&params, state->reportLimit);
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

static void print_group_help(vector_t *groups)
{
    for (size_t i = 0; i < vector_len(groups); i++)
    {
        group_t *group = vector_get(groups, i);
        printf("%s - %s\n", group->name, group->desc);
        for (size_t j = 0; j < vector_len(group->params); j++)
        {
            param_t *param = vector_get(group->params, j);
            char *names = join_names(param);
            printf("  %-20s : %s\n", names, param->desc);
        }
    }
}

static void print_help(const arg_parse_config_t *config, vector_t *defaultGroups)
{
    printf("usage: %s [options & files]\n", config->argv[0]);

    print_group_help(defaultGroups);
    print_group_help(config->groups);
}

static void print_version(const arg_parse_config_t *config)
{
    printf("version: %u.%u.%u\n", VERSION_MAJOR(config->version), VERSION_MINOR(config->version), VERSION_PATCH(config->version));
}

static int process_general_args(const arg_parse_config_t *config, vector_t *groups, argparse_state_t state, argparse_t argparse, report_config_t reportConfig)
{
    int result = end_reports(argparse.reports, "command line parsing", reportConfig);
    if (result != EXIT_OK)
    {
        return result;
    }

    bool printHelp = get_bool_arg(state.printHelp, false);
    bool printVersion = get_bool_arg(state.printVersion, false);

    if (printHelp)
    {
        print_help(config, groups);
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
            .reportConfig = DEFAULT_REPORT_CONFIG
        };

        return result;
    }

    argparse_t argparse = new_argparse(config);

    argparse_state_t state = state_new();

    vector_t *groups = vector_new(2);
    vector_push(&groups, group_general(&state));
    vector_push(&groups, group_reporting(&state));

    add_groups(argparse.params, groups);
    add_groups(argparse.params, config->groups);

    scan_set(scan, &argparse);
    compile_scanner(scan, &kCallbacks);

    argparse_end_flag(&argparse);

    int exitCode = process_general_args(config, groups, state, argparse, reportConfig);

    arg_parse_result_t result = {
        .exitCode = exitCode,
        .params = argparse.params,
        .extra = argparse.extra,
        .reportConfig = reportConfig
    };

    return result;
}
