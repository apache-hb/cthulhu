#pragma once

#include "cthulhu/ast/compile.h"

#include <gmp.h>

typedef struct {
    vector_t *sources; ///< source files

#define COMMAND(name, type, initial, description, ...) type name;
#include "flags.inc"
} commands_t;

int parse_commandline(reports_t *reports, commands_t *commands, int argc, const char **argv);

void cmd_add_file(commands_t *commands, char *path);

#define CMDLTYPE where_t
