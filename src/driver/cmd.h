#pragma once

#include "cthulhu/ast/compile.h"
#include "cthulhu/util/report.h"

#include <gmp.h>

typedef bool cmd_bool_t;
typedef char *cmd_string_t;
typedef int cmd_int_t;

typedef struct {
    reports_t *reports;
    vector_t *sources; ///< source files

#define COMMAND(name, type, initial, description, ...) cmd_##type##_t name;
#include "flags.inc"
} commands_t;

typedef enum {
    // we break style here because of how the command line flags are defined
    // see flags.inc
    CMD_bool,
    CMD_string,
    CMD_int
} flag_type_t;

typedef struct {
    const char *name;
    size_t offset;
    flag_type_t type;
} flag_t;

typedef struct {
    flag_type_t type;
    union {
        char *string;
        mpz_t mpz;
    };
} option_t;

int parse_commandline(reports_t *reports, commands_t *commands, int argc, const char **argv);

void cmd_add_file(commands_t *commands, char *path);
void cmd_set_flag(commands_t *commands, flag_t *flag);
void cmd_set_option(commands_t *commands, flag_t *flag, option_t option);

flag_t *cmd_get_flag(commands_t *commands, const char *str);

option_t option_ident(char *str);
option_t option_number(mpz_t mpz);

#define CMDLTYPE where_t
