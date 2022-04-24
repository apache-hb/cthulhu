#include "cmd.h"

#include "cmd-bison.h"
#include "cmd-flex.h"
#include "cthulhu/ast/compile.h"
#include "cthulhu/driver/driver.h"
#include "cthulhu/util/macros.h"
#include "cthulhu/util/map.h"
#include "cthulhu/util/str.h"
#include "mini-gmp.h"
#include "src/driver/cmd.h"

CT_CALLBACKS(kCallbacks, cmd);

static map_t *offsets = NULL;

static flag_t *flag_new(size_t offset, flag_type_t type, const char *name) {
    flag_t *flag = ctu_malloc(sizeof(flag_t));
    flag->offset = offset;
    flag->type = type;
    flag->name = name;
    return flag;
}

static void cmd_init(void) {
    if (offsets != NULL) {
        return;
    }

    offsets = map_new(64);
#define COMMAND(name, type, initial, description, ...)                             \
    do {                                                                           \
        const char *names[] = __VA_ARGS__;                                         \
        size_t total = sizeof(names) / sizeof(const char *);                       \
        flag_t *flag = flag_new(offsetof(commands_t, name), CMD_##type, names[0]); \
        for (size_t i = 0; i < total; i++) {                                       \
            map_set(offsets, names[i], flag);                                      \
        }                                                                          \
    } while (0);
#include "flags.inc"
}

void cmd_add_file(commands_t *commands, char *path) {
    vector_push(&commands->sources, path);
}

void cmd_set_flag(commands_t *commands, flag_t *flag) {
    if (flag == NULL) {
        return;
    }

    if (flag->type != CMD_bool) {
        report(commands->reports, WARNING, NULL, "flag `%s` requires a value", flag->name);
        return;
    }

    bool *boolean = (bool *)((char *)commands + flag->offset);

    if (*boolean) {
        report(commands->reports, WARNING, NULL, "flag `%s` already set", flag->name);
        return;
    }

    *boolean = true;
}

void cmd_set_option(commands_t *commands, flag_t *flag, option_t option) {
    if (flag == NULL) {
        report(commands->reports, WARNING, NULL, "unknown flag `%s`", flag->name);
        return;
    }

    if (flag->type != option.type) {
        report(commands->reports, WARNING, NULL, "incorrect argument type passed for `%s`", flag->name);
        return;
    }

    void *value = (void *)((char *)commands + flag->offset);
    switch (option.type) {
    case CMD_string:
        *(char **)value = option.string;
        break;
    case CMD_int:
        *(int *)value = mpz_get_si(option.mpz);
        break;
    default:
        cmd_set_flag(commands, flag);
        break;
    }
}

flag_t *cmd_get_flag(commands_t *commands, const char *str) {
    flag_t *flag = map_get(offsets, str);

    if (flag == NULL) {
        report(commands->reports, ERROR, NULL, "unknown flag %s", str);
        return NULL;
    }

    return flag;
}

option_t option_ident(char *str) {
    option_t option = {.type = CMD_string, .string = str};
    return option;
}

option_t option_number(mpz_t mpz) {
    option_t option = {.type = CMD_int};

    mpz_init_set(option.mpz, mpz);
    return option;
}

void cmderror(where_t *where, void *state, scan_t *scan, const char *msg) {
    UNUSED(state);

    report(scan->reports, ERROR, node_new(scan, *where), "%s", msg);
}

static char *join_args(int argc, const char **argv) {
    vector_t *vec = vector_of(argc - 1);
    for (int i = 1; i < argc; i++) {
        vector_set(vec, i - 1, (char *)argv[i]);
    }
    // join with the bell to make the lexers job easier
    return str_join("\b", vec);
}

int parse_commandline(reports_t *reports, commands_t *commands, int argc, const char **argv) {
    cmd_init();

    char *args = join_args(argc, argv);

    scan_t scan = scan_string(reports, "command-line", "<command-line>", args);

    int status = end_reports(reports, DEFAULT_REPORT_LIMIT, "command line parsing");
    if (status != 0) {
        return status;
    }

    commands->sources = vector_new(8);
    commands->reports = reports;

    scan_set(&scan, commands);
    compile_string(&scan, &kCallbacks);

    /* set default values if they werent specified */
    /* isnt it great that everything converts to 0 in c */
#define COMMAND(name, type, initial, description, ...) \
    do {                                               \
        if (commands->name == 0) {                     \
            commands->name = initial;                  \
        }                                              \
    } while (0);
#include "flags.inc"

    status = end_reports(reports, commands->warningLimit, "command line parsing");
    if (status != 0) {
        return status;
    }

    return 0;
}
