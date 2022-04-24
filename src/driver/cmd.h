#pragma once

#include "cthulhu/ast/compile.h"
#include "cthulhu/util/report.h"

#include <gmp.h>

typedef enum {
    FLAG_BOOL,
    FLAG_STRING,
    FLAG_INT,

    FLAG_NONE
} flag_type_t;

typedef struct {
    const char *name;
    bool *setByUser;
    flag_type_t type;
    void *data;
} flag_t;

typedef struct {
    reports_t *reports;
    flag_t currentFlag;

    vector_t *files; ///< all files provided by the user
                     ///  can be source and library files

#define TYPE_BOOL bool
#define TYPE_INT int
#define TYPE_STRING char *
#define COMMAND(name, type, initial, description, ...) type name; bool name##SetByUser;
#include "flags.inc"
} commands_t;

int parse_commandline(reports_t *reports, commands_t *commands, int argc, const char **argv);

void cmd_begin_flag(commands_t *commands, const char *name);

void cmd_push_int(commands_t *commands, mpz_t value);
void cmd_push_str(commands_t *commands, char *value);

#define CMDLTYPE where_t
