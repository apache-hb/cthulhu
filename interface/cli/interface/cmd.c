#include "cmd.h"

#include "cmd-bison.h"
#include "cmd-flex.h"
#include "cthulhu/ast/compile.h"
#include "cthulhu/interface/runtime.h"
#include "cthulhu/util/macros.h"
#include "cthulhu/util/str.h"
#include "cthulhu/util/util.h"
#include <gmp.h>

CT_CALLBACKS(kCallbacks, cmd);

static bool flag_matches(commands_t *commands, flag_t currentFlag, const char *flag, size_t total, const char **names)
{
    size_t flagLen = strlen(flag);
    for (size_t i = 0; i < total; i++)
    {
        if (str_startswith(flag, names[i]))
        {
            size_t nameLen = strlen(names[i]);
            commands->currentFlag = currentFlag;

            if (flagLen > nameLen)
            {
                char *restOfFlag = ctu_strdup(flag + nameLen);
                cmd_push_str(commands, restOfFlag);
            }
            return true;
        }
    }

    return false;
}

static flag_t flag_new(const char *name, bool *setByUser, flag_type_t type, void *data)
{
    flag_t flag = {.name = name, .setByUser = setByUser, .type = type, .data = data};
    return flag;
}

static flag_t flag_empty(void)
{
    return flag_new(NULL, NULL, FLAG_NONE, NULL);
}

static bool check_and_set_flag(commands_t *commands, flag_t flag)
{
    if (*flag.setByUser)
    {
        report(commands->reports, WARNING, node_invalid(), "flag `%s` already set", flag.name);
        return true;
    }

    *flag.setByUser = true;
    return false;
}

static bool should_skip_flag(flag_t flag)
{
    return flag.name == NULL && flag.type == FLAG_NONE;
}

static const char *kFlagTypes[FLAG_NONE] = {
    [FLAG_BOOL] = "positional",
    [FLAG_INT] = "integer",
    [FLAG_STRING] = "string",
};

static flag_t pop_current_flag(commands_t *commands, flag_type_t type)
{
    flag_t flag = commands->currentFlag;
    commands->currentFlag = flag_empty();

    if (flag.name == NULL)
    {
        return flag_empty();
    }

    /* if this is a positional flag, we should set it when we pop it */
    if (flag.type == FLAG_BOOL)
    {
        check_and_set_flag(commands, flag);
        *(bool *)flag.data = true;
        return flag_empty();
    }

    if (flag.type != type)
    {
        report(commands->reports, WARNING, node_invalid(), "flag `%s` requires a %s, but was treated as a %s flag", flag.name,
               kFlagTypes[flag.type], kFlagTypes[type]);
        return flag_empty();
    }

    return flag;
}

void cmd_begin_flag(commands_t *commands, const char *flag)
{
    pop_current_flag(commands, FLAG_BOOL);

#define TYPE_BOOL FLAG_BOOL
#define TYPE_STRING FLAG_STRING
#define TYPE_INT FLAG_INT
#define COMMAND(name, type, initial, description, ...)                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        const char *names[] = __VA_ARGS__;                                                                             \
        size_t total = sizeof(names) / sizeof(const char *);                                                           \
        flag_t currentFlag = flag_new(names[0], &commands->name##SetByUser, type, &commands->name);                    \
        if (flag_matches(commands, currentFlag, flag, total, names))                                                   \
        {                                                                                                              \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0);
#include "flags.inc"

    report(commands->reports, WARNING, node_invalid(), "unknown flag `%s`", flag);
}

void cmd_push_int(commands_t *commands, mpz_t value)
{
    flag_t flag = pop_current_flag(commands, FLAG_INT);
    if (should_skip_flag(flag))
    {
        return;
    }
    if (check_and_set_flag(commands, flag))
    {
        return;
    }

    (*(int *)flag.data) = mpz_get_si(value);
}

void cmd_push_str(commands_t *commands, char *value)
{
    flag_t flag = pop_current_flag(commands, FLAG_STRING);

    // if the string is set without a flag its a file
    if (flag.type == FLAG_NONE)
    {
        vector_push(&commands->files, value);
        return;
    }

    if (check_and_set_flag(commands, flag))
    {
        return;
    }

    *(char **)flag.data = value;
}

void cmderror(where_t *where, void *state, scan_t *scan, const char *msg)
{
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
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

int parse_commandline(reports_t *reports, commands_t *commands, int argc, const char **argv)
{
    char *args = join_args(argc, argv);

    scan_t scan = scan_string(reports, "command-line", "<command-line>", args);

    report_config_t reportSettings = {.limit = DEFAULT_REPORT_LIMIT, .warningsAreErrors = false};

    int status = end_reports(reports, "command line parsing", reportSettings);
    if (status != 0)
    {
        return status;
    }

    /* setup the commands data */
#define COMMAND(name, type, initial, description, ...)                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        commands->name##SetByUser = false;                                                                             \
        commands->name = initial;                                                                                      \
    } while (0);
#include "flags.inc"

    commands->reports = reports;
    commands->currentFlag = flag_empty();
    commands->files = vector_new(8);

    scan_set(&scan, commands);
    compile_string(&scan, &kCallbacks);

    pop_current_flag(commands, FLAG_BOOL);

    reportSettings.limit = commands->warningLimit;
    reportSettings.warningsAreErrors = commands->warningsAsErrors;

    status = end_reports(reports, "command line parsing", reportSettings);
    if (status != 0)
    {
        return status;
    }

    return 0;
}
