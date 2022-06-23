#include "argparse/argparse.h"
#include "argparse/common.h"

#include "base/macros.h"
#include "base/util.h"

#include "std/map.h"
#include "std/vector.h"
#include "std/str.h"

#include "scan/compile.h"

#include "report/report.h"

#include "cmd-bison.h"
#include "cmd-flex.h"

#include <limits.h>

CT_CALLBACKS(kCallbacks, cmd);

void cmderror(where_t *where, void *state, scan_t scan, const char *msg)
{
    UNUSED(state);

    report(scan_reports(scan), ERROR, node_new(scan, *where), "%s", msg);
}

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

static param_t *kHelpParam = NULL;
static param_t *kVersionParam = NULL;
static param_t *kVerboseLoggingParam = NULL;
static param_t *kFatalWarningsParam = NULL;
static param_t *kReportLimitParam = NULL;

static group_t *kGeneralGroup = NULL;
static group_t *kReportingGroup = NULL;

static param_t *new_param(param_kind_t kind, const char *desc, const char **names, size_t total)
{
    param_t *result = ctu_malloc(sizeof(param_t));
    result->kind = kind;
    result->description = desc;
    result->names = names;
    result->totalNames = total;
    return result;
}

static void argparse_init(void)
{
    static bool init = false;
    if (init)
    {
        init = true;
        return;
    }

    kHelpParam = bool_param("print this help message", kHelpArgs, TOTAL_HELP_ARGS);
    kVersionParam = bool_param("print the version number", kVersionArgs, TOTAL_VERSION_ARGS);

    kVerboseLoggingParam = bool_param("enable verbose logging", kVerboseLoggingArgs, TOTAL_VERBOSE_LOGGING_ARGS);
    kFatalWarningsParam = bool_param("enable fatal warnings", kFatalWarningsArgs, TOTAL_FATAL_WARNINGS_ARGS);
    kReportLimitParam = int_param("set the report limit", kReportLimitArgs, TOTAL_REPORT_LIMIT_ARGS);

    vector_t *generalParams = vector_new(4);
    vector_push(&generalParams, kHelpParam);
    vector_push(&generalParams, kVersionParam);
    kGeneralGroup = new_group("general", "general options", generalParams);

    vector_t *reportingParams = vector_new(4);
    vector_push(&reportingParams, kVerboseLoggingParam);
    vector_push(&reportingParams, kFatalWarningsParam);
    vector_push(&reportingParams, kReportLimitParam);
    kReportingGroup = new_group("reporting", "reporting options", reportingParams);
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

static argparse_t new_argparse(const argparse_config_t *config)
{
    argparse_t result = {
        .exitCode = INT_MAX,
        
        .verboseEnabled = false,
        .reportLimit = 20,
        .warningsAsErrors = false,

        .params = map_optimal(total_arg_names(config->groups)),

        .files = vector_new(config->argc - 1),

        .reports = config->reports,
        .currentName = NULL,
        .currentArg = NULL
    };

    return result;
}

static void add_arg(argparse_t *ctx, param_t *param)
{
    arg_t *result = ctu_malloc(sizeof(arg_t));
    result->kind = param->kind;
    result->setByUser = false;
    
    map_set_ptr(ctx->params, param, result);
}

static void add_group(argparse_t *argparse, group_t *group)
{
    for (size_t i = 0; i < vector_len(group->params); i++)
    {
        param_t *param = vector_get(group->params, i);
        add_arg(argparse, param);
    }
}

static void add_groups(argparse_t *argparse, vector_t *groups)
{
    for (size_t i = 0; i < vector_len(groups); i++)
    {
        group_t *group = vector_get(groups, i);
        add_group(argparse, group);
    }
}

param_t *int_param(const char *desc, const char **names, size_t total)
{
    return new_param(PARAM_INT, desc, names, total);
}

param_t *string_param(const char *desc, const char **names, size_t total)
{
    return new_param(PARAM_STRING, desc, names, total);
}

param_t *bool_param(const char *desc, const char **names, size_t total)
{
    return new_param(PARAM_BOOL, desc, names, total);
}

group_t *new_group(const char *name, const char *desc, vector_t *params) 
{
    group_t *result = ctu_malloc(sizeof(group_t));
    result->name = name;
    result->description = desc;
    result->params = params;
    return result;
}

long get_digit(const argparse_t *argparse, const param_t *arg, long other) 
{
    arg_t *associated = map_get_ptr(argparse->params, arg);
    CTASSERT(associated != NULL, "parameter not found");
    CTASSERT(associated->kind == PARAM_INT, "parameter is not an integer");

    if (!associated->setByUser)
    {
        return other;
    }

    return associated->digit;
}

const char *get_string(const argparse_t *argparse, const param_t *arg, const char *other)
{
    arg_t *associated = map_get_ptr(argparse->params, arg);
    CTASSERT(associated != NULL, "parameter not found");
    CTASSERT(associated->kind == PARAM_STRING, "parameter is not a string");

    if (!associated->setByUser)
    {
        return other;
    }

    return associated->string;
}

bool get_bool(const argparse_t *argparse, const param_t *arg, bool other)
{
    arg_t *associated = map_get_ptr(argparse->params, arg);
    CTASSERT(associated != NULL, "parameter not found");
    CTASSERT(associated->kind == PARAM_BOOL, "parameter is not a boolean");

    if (!associated->setByUser)
    {
        return other;
    }

    return associated->boolean;
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

static void print_group_help(group_t *group)
{
    printf("%s - %s\n", group->name, group->description);
    for (size_t j = 0; j < vector_len(group->params); j++)
    {
        param_t *param = vector_get(group->params, j);
        char *names = join_names(param);
        printf("  %-20s : %s\n", names, param->description);
    }
}

static void print_help(const argparse_config_t *config)
{
    printf("usage: %s [options & files]\n", config->argv[0]);

    print_group_help(kGeneralGroup);
    print_group_help(kReportingGroup);

    for (size_t i = 0; i < vector_len(config->groups); i++)
    {
        group_t *group = vector_get(config->groups, i);
        print_group_help(group);
    }
}

static void print_version(const argparse_config_t *config)
{
    printf("version: %u.%u.%u\n", VERSION_MAJOR(config->version), VERSION_MINOR(config->version), VERSION_PATCH(config->version));
}

static int process_general_args(const argparse_config_t *config, argparse_t *argparse)
{
    report_config_t reportConfig = {
        .limit = argparse->reportLimit,
        .warningsAreErrors = argparse->warningsAsErrors
    };

    int result = end_reports(argparse->reports, "command line parsing", reportConfig);
    if (result != EXIT_OK)
    {
        return result;
    }

    bool printHelp = get_bool(argparse, kHelpParam, false);
    bool printVersion = get_bool(argparse, kVersionParam, false);

    if (printHelp)
    {
        print_help(config);
    }

    if (printVersion)
    {
        print_version(config);
    }

    return INT_MAX;
}

bool should_exit(const argparse_t *argparse)
{
    return argparse->exitCode != INT_MAX;
}

argparse_t parse_args(const argparse_config_t *config)
{
    argparse_init();
    argparse_t argparse = new_argparse(config);

    char *args = join_args(config->argc, config->argv);

    scan_t scan = scan_string(config->reports, "command-parser", "<command-line>", args);

    report_config_t reportConfig = {
        .limit = 20,
        .warningsAreErrors = false,
    };

    int status = end_reports(config->reports, "command line parsing", reportConfig);
    if (status != 0)
    {
        argparse.exitCode = status;
        return argparse;
    }

    add_group(&argparse, kGeneralGroup);
    add_group(&argparse, kReportingGroup);
    add_groups(&argparse, config->groups);

    scan_set(scan, &argparse);
    compile_scanner(scan, &kCallbacks);

    argparse_end_flag(&argparse);

    int exitCode = process_general_args(config, &argparse);

    argparse.exitCode = exitCode;
    return argparse;
}